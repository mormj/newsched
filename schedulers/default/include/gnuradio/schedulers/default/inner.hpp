//#include <gnuradio/scheduler.hpp>
#include <gnuradio/scheduler.hpp>
// #include <boost/circular_buffer.hpp>
#include <gnuradio/concurrent_queue.hpp>
#include <gnuradio/domain_adapter.hpp>
#include <gnuradio/scheduler_message.hpp>
#include <gnuradio/simplebuffer.hpp>
#include <map>
#include <thread> // std::thread

namespace gr {

namespace schedulers {

class scheduler_default_inner : public scheduler
{
private:
    std::string _name;

public:
    scheduler_sync* sched_sync;
    const int s_fixed_buf_size;
    static const int s_min_items_to_process = 1;
    const int s_max_buf_items; // = s_fixed_buf_size / 2;

    typedef std::shared_ptr<scheduler_default_inner> sptr;

    scheduler_default_inner(const std::string name = "default_inner",
                            const unsigned int fixed_buf_size = 8192)
        : scheduler(name),
          s_fixed_buf_size(fixed_buf_size),
          s_max_buf_items(fixed_buf_size / 2)
    {
    }
    ~scheduler_default_inner(){

    };

    void initialize(flat_graph_sptr fg,
                    flowgraph_monitor_sptr fgmon,
                    block_scheduler_map block_sched_map)
    {
        d_fg = fg;
        d_fgmon = fgmon;
        d_block_sched_map = block_sched_map;

        // if (fg->is_flat())  // flatten

        // not all edges may be used
        for (auto e : fg->edges()) {
            // every edge needs a buffer
            d_edge_catalog[e.identifier()] = e;

            auto src_da_cast = std::dynamic_pointer_cast<domain_adapter>(e.src().node());
            auto dst_da_cast = std::dynamic_pointer_cast<domain_adapter>(e.dst().node());

            if (src_da_cast != nullptr) {
                if (src_da_cast->buffer_location() == buffer_location_t::LOCAL) {
                    auto buf = simplebuffer::make(s_fixed_buf_size, e.itemsize());
                    src_da_cast->set_buffer(buf);
                    auto tmp = std::dynamic_pointer_cast<buffer>(src_da_cast);
                    d_edge_buffers[e.identifier()] = tmp;
                } else {
                    d_edge_buffers[e.identifier()] =
                        std::dynamic_pointer_cast<buffer>(src_da_cast);
                }
            } else if (dst_da_cast != nullptr) {
                if (dst_da_cast->buffer_location() == buffer_location_t::LOCAL) {
                    auto buf = simplebuffer::make(s_fixed_buf_size, e.itemsize());
                    dst_da_cast->set_buffer(buf);
                    auto tmp = std::dynamic_pointer_cast<buffer>(dst_da_cast);
                    d_edge_buffers[e.identifier()] = tmp;
                } else {
                    d_edge_buffers[e.identifier()] =
                        std::dynamic_pointer_cast<buffer>(dst_da_cast);
                }

            } else {
                d_edge_buffers[e.identifier()] =
                    simplebuffer::make(s_fixed_buf_size, e.itemsize());
            }

            d_edge_buffers[e.identifier()]->set_name(e.identifier());
        }

        for (auto& b : fg->calc_used_blocks()) {
            b->set_scheduler(base());
            d_blocks.push_back(b);

            port_vector_t input_ports = b->input_stream_ports();
            port_vector_t output_ports = b->output_stream_ports();

            unsigned int num_input_ports = input_ports.size();
            unsigned int num_output_ports = output_ports.size();

            for (auto p : input_ports) {
                d_block_buffers[p] = std::vector<buffer_sptr>{};
                edge_vector_t ed = d_fg->find_edge(p);
                for (auto e : ed)
                    d_block_buffers[p].push_back(d_edge_buffers[e.identifier()]);
            }

            for (auto p : output_ports) {
                d_block_buffers[p] = std::vector<buffer_sptr>{};
                edge_vector_t ed = d_fg->find_edge(p);
                for (auto e : ed)
                    d_block_buffers[p].push_back(d_edge_buffers[e.identifier()]);
            }
        }
    }
    void start()
    {
        for (auto& b : d_blocks) {
            b->start();
        }
        d_thread = std::thread(thread_body, this);

        push_message(std::make_shared<scheduler_action>(
            scheduler_action(scheduler_action_t::NOTIFY_ALL)));
    }
    void stop()
    {
        d_thread_stopped = true;
        d_thread.join();
        for (auto& b : d_blocks) {
            b->stop();
        }
    }
    void wait()
    {
        d_thread.join();
        for (auto& b : d_blocks) {
            b->done();
        }
    }
    void run()
    {
        start();
        wait();
    }

    std::map<uint32_t, scheduler_iteration_status>
    run_one_iteration(std::vector<block_sptr> blocks = std::vector<block_sptr>())
    {
        std::map<uint32_t, scheduler_iteration_status> per_block_status;

        // If no blocks are specified for the iteration, then run over all the blocks in
        // the default ordering
        if (blocks.empty()) {
            blocks = d_blocks;
        }

        for (auto const& b : blocks) { // TODO - order the blocks

            std::vector<block_work_input> work_input;   //(num_input_ports);
            std::vector<block_work_output> work_output; //(num_output_ports);

            std::vector<buffer_sptr> bufs;
            // for each input port of the block
            bool ready = true;
            for (auto p : b->input_stream_ports()) {
                auto p_buf = d_block_buffers[p][0];


                buffer_info_t read_info;
                ready = p_buf->read_info(read_info);
                gr_log_debug(
                    _debug_logger, "read_info {} - {}", b->alias(), read_info.n_items);

                if (!ready)
                    break;

                bufs.push_back(p_buf);

                if (read_info.n_items < s_min_items_to_process) {
                    ready = false;
                    break;
                }

                auto tags = p_buf->get_tags(read_info.n_items);
                work_input.push_back(block_work_input(
                    read_info.n_items, read_info.total_items, read_info.ptr, tags));
            }

            if (!ready) {
                per_block_status[b->id()] = scheduler_iteration_status::BLKD_IN;
                continue;
            }

            // for each output port of the block
            for (auto p : b->output_stream_ports()) {

                // When a block has multiple output buffers, it adds the restriction
                // that the work call can only produce the minimum available across
                // the buffers.

                int max_output_buffer = std::numeric_limits<int>::max();

                void* write_ptr = nullptr;
                uint64_t nitems_written;
                for (auto p_buf : d_block_buffers[p]) {
                    buffer_info_t write_info;
                    ready = p_buf->write_info(write_info);
                    gr_log_debug(_debug_logger,
                                 "write_info {} - {} @ {} {}",
                                 b->alias(),
                                 write_info.n_items,
                                 write_info.ptr,
                                 write_info.item_size);
                    if (!ready)
                        break;
                    bufs.push_back(p_buf);

                    if (write_info.n_items <= s_max_buf_items) {
                        ready = false;
                        break;
                    }

                    int tmp_buf_size = write_info.n_items;
                    if (tmp_buf_size < max_output_buffer - 1)
                        max_output_buffer = tmp_buf_size;

                    // store the first buffer
                    if (!write_ptr) {
                        write_ptr = write_info.ptr;
                        nitems_written = write_info.total_items;
                    }
                }

                max_output_buffer = std::min(max_output_buffer, s_max_buf_items);
                std::vector<tag_t> tags; // needs to be associated with edge buffers

                work_output.push_back(block_work_output(
                    max_output_buffer, nitems_written, write_ptr, tags));
            }

            if (!ready) {
                per_block_status[b->id()] = scheduler_iteration_status::BLKD_OUT;
                continue;
            }

            if (ready) {
                gr_log_debug(_debug_logger, "do_work for {}", b->alias());
                work_return_code_t ret = b->do_work(work_input, work_output);
                gr_log_debug(_debug_logger, "do_work returned {}", ret);

                if (ret == work_return_code_t::WORK_DONE) {
                    per_block_status[b->id()] = scheduler_iteration_status::DONE;
                } else if (ret == work_return_code_t::WORK_OK) {
                    per_block_status[b->id()] = scheduler_iteration_status::READY;
                }
                // TODO - handle READY_NO_OUTPUT

                if (ret == work_return_code_t::WORK_OK ||
                    ret == work_return_code_t::WORK_DONE) {


                    int input_port_index = 0;
                    for (auto p : b->input_stream_ports()) {
                        auto p_buf =
                            d_block_buffers[p][0]; // only one buffer per input port

                        // Pass the tags according to TPP
                        if (b->tag_propagation_policy() ==
                            tag_propagation_policy_t::TPP_ALL_TO_ALL) {
                            int output_port_index = 0;
                            for (auto op : b->output_stream_ports()) {
                                for (auto p_out_buf : d_block_buffers[op]) {
                                    p_out_buf->add_tags(
                                        work_output[output_port_index].n_produced,
                                        work_input[input_port_index].tags);
                                }
                                output_port_index++;
                            }
                        } else if (b->tag_propagation_policy() ==
                                   tag_propagation_policy_t::TPP_ONE_TO_ONE) {
                            int output_port_index = 0;
                            for (auto op : b->output_stream_ports()) {
                                if (output_port_index == input_port_index) {
                                    for (auto p_out_buf : d_block_buffers[op]) {
                                        p_out_buf->add_tags(
                                            work_output[output_port_index].n_produced,
                                            work_input[input_port_index].tags);
                                    }
                                }
                                output_port_index++;
                            }
                        }

                        gr_log_debug(_debug_logger,
                                     "post_read {} - {}",
                                     b->alias(),
                                     work_input[input_port_index].n_consumed);

                        p_buf->post_read(work_input[input_port_index].n_consumed);
                        gr_log_debug(_debug_logger, ".");
                        input_port_index++;
                    }

                    int output_port_index = 0;
                    for (auto p : b->output_stream_ports()) {
                        int j = 0;
                        for (auto p_buf : d_block_buffers[p]) {
                            if (j > 0) {
                                gr_log_debug(_debug_logger,
                                             "copy_items {} - {}",
                                             b->alias(),
                                             work_output[output_port_index].n_produced);
                                p_buf->copy_items(
                                    d_block_buffers[p][0],
                                    work_output[output_port_index].n_produced);
                                gr_log_debug(_debug_logger, ".");
                            }
                            j++;
                        }
                        for (auto p_buf : d_block_buffers[p]) {
                            // Add the tags that were collected in the work() call
                            if (!work_output[output_port_index].tags.empty()) {
                                p_buf->add_tags(work_output[output_port_index].n_produced,
                                                work_output[output_port_index].tags);
                            }

                            gr_log_debug(_debug_logger,
                                         "post_write {} - {}",
                                         b->alias(),
                                         work_output[output_port_index].n_produced);
                            p_buf->post_write(work_output[output_port_index].n_produced);
                            gr_log_debug(_debug_logger, ".");
                        }
                        output_port_index++;
                    }
                }
            }
        }

        return per_block_status;
    }


private:
    flat_graph_sptr d_fg;
    block_scheduler_map
        d_block_sched_map; // map of block ids to scheduler interfaces / adapters
    flowgraph_monitor_sptr d_fgmon;
    std::vector<block_sptr> d_blocks;
    std::map<std::string, edge> d_edge_catalog;
    std::map<std::string, buffer_sptr> d_edge_buffers;
    std::map<port_sptr, std::vector<buffer_sptr>> d_block_buffers;
    std::thread d_thread;
    bool d_thread_stopped = false;


    static void thread_body(scheduler_default_inner* top)
    {
        int num_empty = 0;
        bool work_done = false;
        top->set_state(scheduler_state::WORKING);
        gr_log_info(top->_logger, "starting thread");
        while (!top->d_thread_stopped) {

            // try to pop messages off the queue
            scheduler_message_sptr msg;
            if (top->pop_message(msg)) // this blocks
            {
                std::cout << typeid(msg).name() << std::endl;
                switch (msg->type()) {
                case scheduler_message_t::SCHEDULER_ACTION: {
                    // Notification that work needs to be done
                    // either from runtime or upstream or downstream or from self

                    auto action = std::static_pointer_cast<scheduler_action>(msg);
                    std::cout << "action: " << (int)action->action() << std::endl;

                    switch (action->action()) {
                    case scheduler_action_t::NOTIFY_OUTPUT:
                    case scheduler_action_t::NOTIFY_INPUT:
                    case scheduler_action_t::NOTIFY_ALL: {

                        auto s = top->run_one_iteration();
                        for (auto elem : s) {
                            std::cout << elem.first << " " << (int)elem.second
                                      << std::endl;
                        }

                        // Based on state of the run_one_iteration, do things
                        // If any of the blocks are done, notify the flowgraph monitor
                        for (auto elem : s) {
                            if (elem.second == scheduler_iteration_status::DONE) {
                                top->d_fgmon->push_message(
                                    fg_monitor_message(fg_monitor_message_t::DONE));
                            }
                        }

                        // If work was done in this iteration, tell my own thread
                        // to go for it again
                        bool notify_myself = false;
                        for (auto elem : s) {

                            if (elem.second == scheduler_iteration_status::READY) {
                                notify_myself = true;
                            }
                        }
                        if (notify_myself) {
                            top->push_message(std::make_shared<scheduler_action>(
                                scheduler_action(scheduler_action_t::NOTIFY_ALL)));
                        }
                    } break;
                    case scheduler_action_t::DONE:
                        // fgmon says that we need to be done, wrap it up
                        // each scheduler could handle this in a different way
                        top->d_fgmon->push_message(
                            fg_monitor_message(fg_monitor_message_t::FLUSHED));
                        break;
                    case scheduler_action_t::EXIT:
                        // fgmon says that we need to be done, wrap it up
                        // each scheduler could handle this in a different way
                        top->d_thread_stopped = true;
                        break;
                    }
                    break;
                }
                case scheduler_message_t::PARAMETER_QUERY: {
                    // Query the state of a parameter on a block

                    // auto query = std::static_pointer_cast<parameter_query>(msg);
                    //

                    // std::cout << typeid(query).name() << std::endl;

                } break;
                }
                }
            }
        }
    }; // namespace schedulers
} // namespace schedulers
} // namespace schedulers
