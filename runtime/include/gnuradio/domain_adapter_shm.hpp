#pragma once

#include <gnuradio/domain_adapter.hpp>
#include <condition_variable>
#include <mutex>
#include <atomic>
namespace gr {

class shm_sync
{
public:
    std::condition_variable cv;
    std::mutex mtx;
    std::atomic<int> ready = 0;

    da_request_t request;
    da_response_t response;
    int num_items;
    buffer_info_t info;

    shm_sync() {}
    static std::shared_ptr<shm_sync> make()
    {
        return std::shared_ptr<shm_sync>(new shm_sync());
    }
};

typedef std::shared_ptr<shm_sync> shm_sync_sptr;
/**
 * @brief Uses a shared memory and a condition variable to communicate buffer pointers
 * between domains
 *
 */
class domain_adapter_shm_svr : public domain_adapter
{
private:
    bool d_connected = false;
    std::thread d_thread;

public:
    shm_sync_sptr p_sync;
    typedef std::shared_ptr<domain_adapter_shm_svr> sptr;
    static sptr make(shm_sync_sptr sync, port_sptr other_port)
    {
        auto ptr = std::make_shared<domain_adapter_shm_svr>(domain_adapter_shm_svr(sync));

        ptr->add_port(port_base::make("output",
                                      port_direction_t::OUTPUT,
                                      other_port->data_type(),
                                      port_type_t::STREAM,
                                      other_port->dims()));

        ptr->start_thread(ptr); // start thread with reference to shared pointer

        return ptr;
    }

    domain_adapter_shm_svr(shm_sync_sptr sync)
        : domain_adapter(buffer_location_t::LOCAL), p_sync(sync)
    {
    }

    void start_thread(sptr ptr) { d_thread = std::thread(run_thread, ptr); }

    static void run_thread(sptr top)
    {
        while (true) {
            {
                std::unique_lock<std::mutex> l(top->p_sync->mtx);
                //std::cout << "svr unlock" << std::endl;
                top->p_sync->cv.wait(l, [top]{return top->p_sync->ready == 1;});
                //std::cout << "svr out of wait" << std::endl;

                switch (top->p_sync->request) {
                case da_request_t::CANCEL:
                    //std::cout << "svr CANCEL" << std::endl;
                    top->buffer()->cancel();
                    top->p_sync->response = da_response_t::OK;
                    break;
                case da_request_t::WRITE_INFO:
                    //std::cout << "svr WRITE_INFO" << std::endl;
                    if (top->buffer()->write_info(top->p_sync->info)) {
                        top->p_sync->response = da_response_t::OK;
                    } else {
                        top->p_sync->response = da_response_t::ERROR;
                    }
                    break;
                case da_request_t::READ_INFO:
                    //std::cout << "svr READ_INFO" << std::endl;
                    if (top->buffer()->read_info(top->p_sync->info)) {
                        top->p_sync->response = da_response_t::OK;
                    } else {
                        top->p_sync->response = da_response_t::ERROR;
                    }
                    break;
                case da_request_t::POST_WRITE:
                    //std::cout << "svr POST_WRITE" << std::endl;
                    top->buffer()->post_write(top->p_sync->num_items);
                    top->p_sync->response = da_response_t::OK;
                    break;
                case da_request_t::POST_READ:
                    //std::cout << "svr POST_READ" << std::endl;
                    top->buffer()->post_read(top->p_sync->num_items);
                    top->p_sync->response = da_response_t::OK;
                    break;
                }

                //std::cout << "svr out of switch" << std::endl;
                l.unlock();
                //std::cout << "svr notify_one" << std::endl;
                top->p_sync->ready = 2;
                top->p_sync->cv.notify_one();
            }
        }
    }

    virtual void* read_ptr() { return nullptr; }
    virtual void* write_ptr() { return nullptr; }

    virtual bool read_info(buffer_info_t& info) { return _buffer->read_info(info); }
    virtual bool write_info(buffer_info_t& info) { return _buffer->write_info(info); }
    virtual void cancel() { _buffer->cancel(); }

    virtual void post_read(int num_items) { return _buffer->post_read(num_items); }
    virtual void post_write(int num_items) { return _buffer->post_write(num_items); }

    // This is not valid for all buffers, e.g. domain adapters
    virtual void copy_items(buffer_sptr from, int nitems) {}
};


class domain_adapter_shm_cli : public domain_adapter
{
private:
    shm_sync_sptr p_sync;

public:
    typedef std::shared_ptr<domain_adapter_shm_cli> sptr;
    static sptr make(shm_sync_sptr sync, port_sptr other_port)
    {
        auto ptr = std::make_shared<domain_adapter_shm_cli>(domain_adapter_shm_cli(sync));

        // Type of port is not known at compile time
        ptr->add_port(port_base::make("input",
                                      port_direction_t::INPUT,
                                      other_port->data_type(),
                                      port_type_t::STREAM,
                                      other_port->dims()));

        return ptr;
    }
    domain_adapter_shm_cli(shm_sync_sptr sync)
        : domain_adapter(buffer_location_t::LOCAL), p_sync(sync)
    {
    }

    virtual void* read_ptr() { return nullptr; }
    virtual void* write_ptr() { return nullptr; }

    // virtual int capacity() = 0;
    // virtual int size() = 0;

    virtual bool read_info(buffer_info_t& info)
    {
        {
            // std::lock_guard<std::mutex> l(p_sync->mtx);
            //std::cout << "read_info unlock" << std::endl;
            p_sync->request = da_request_t::READ_INFO;
            p_sync->ready = 1;
        }

        //std::cout << "read_info notify_one" << std::endl;
        p_sync->cv.notify_one();

        {
            std::unique_lock<std::mutex> l(p_sync->mtx);
            //std::cout << "read_info wait" << std::endl;
            p_sync->cv.wait(l, [this]{return p_sync->ready == 2;});

            if (p_sync->response == da_response_t::OK) {
                info = p_sync->info;
                p_sync->ready = 0;
                return true;
            } else {
                p_sync->ready = 0;
                return false;
            }
        }
    }
    virtual bool write_info(buffer_info_t& info)
    {
        {
            // std::lock_guard<std::mutex> l(p_sync->mtx);
            //std::cout << "write_info unlock" << std::endl;
            p_sync->request = da_request_t::WRITE_INFO;
            p_sync->ready = 1;
        }
        //std::cout << "write_info notify_one" << std::endl;
        p_sync->cv.notify_one();

        {
            std::unique_lock<std::mutex> l(p_sync->mtx);
            //std::cout << "write_info wait" << std::endl;
            p_sync->cv.wait(l, [this]{return p_sync->ready == 2;});

            if (p_sync->response == da_response_t::OK) {
                info = p_sync->info;
                p_sync->ready = 0;
                return true;
            } else {
                p_sync->ready = 0;
                return false;
            }
        }
    }
    virtual void cancel()
    {
        {
            // std::lock_guard<std::mutex> l(p_sync->mtx);
            //std::cout << "cancel unlock" << std::endl;
            p_sync->request = da_request_t::CANCEL;
            p_sync->ready = 1;
        }
        //std::cout << "cancel notify_one" << std::endl;
        p_sync->cv.notify_one();

        {
            std::unique_lock<std::mutex> l(p_sync->mtx);
            //std::cout << "cancel wait" << std::endl;
            p_sync->cv.wait(l, [this]{return p_sync->ready == 2;});
            p_sync->ready = 0;
        }
    }

    virtual void post_read(int num_items)
    {
        {
            // std::lock_guard<std::mutex> l(p_sync->mtx);
            //std::cout << "post_read unlock" << std::endl;
            p_sync->request = da_request_t::POST_READ;
            p_sync->num_items = num_items;
            p_sync->ready = 1;
        }
        //std::cout << "post_read notify_one" << std::endl;
        p_sync->cv.notify_one();

        {
            std::unique_lock<std::mutex> l(p_sync->mtx);
            //std::cout << "post_read wait" << std::endl;
            p_sync->cv.wait(l, [this]{return p_sync->ready == 2;});
            p_sync->ready = 0;
        }
    }
    virtual void post_write(int num_items)
    {
        {
            // std::lock_guard<std::mutex> l(p_sync->mtx);
            //std::cout << "post_write unlock" << std::endl;
            p_sync->request = da_request_t::POST_WRITE;
            p_sync->num_items = num_items;
            p_sync->ready = 1;
        }
        //std::cout << "post_write notify_one" << std::endl;
        p_sync->cv.notify_one();

        {
            std::unique_lock<std::mutex> l(p_sync->mtx);
            //std::cout << "post_write wait" << std::endl;
            p_sync->cv.wait(l, [this]{return p_sync->ready == 2;});
            p_sync->ready = 0;
        }
    }

    // This is not valid for all buffers, e.g. domain adapters
    // Currently domain adapters require fanout, and cannot copy from a shared output
    // across multiple domains
    virtual void copy_items(buffer_sptr from, int nitems) {}
};


class domain_adapter_shm_conf : public domain_adapter_conf
{
public:
    typedef std::shared_ptr<domain_adapter_shm_conf> sptr;
    static sptr make(buffer_preference_t buf_pref = buffer_preference_t::DOWNSTREAM)
    {
        return std::make_shared<domain_adapter_shm_conf>(
            domain_adapter_shm_conf(buf_pref));
    }

    domain_adapter_shm_conf(buffer_preference_t buf_pref) : domain_adapter_conf(buf_pref)
    {
    }

    virtual std::pair<domain_adapter_sptr, domain_adapter_sptr>
    make_domain_adapter_pair(port_sptr upstream_port, port_sptr downstream_port)
    {
        auto shm_sync = shm_sync::make();

        if (_buf_pref == buffer_preference_t::DOWNSTREAM) {
            auto upstream_adapter = domain_adapter_shm_cli::make(shm_sync, upstream_port);
            auto downstream_adapter =
                domain_adapter_shm_svr::make(shm_sync, downstream_port);

            return std::make_pair(upstream_adapter, downstream_adapter);
        } else {
            auto downstream_adapter =
                domain_adapter_shm_cli::make(shm_sync, upstream_port);
            auto upstream_adapter =
                domain_adapter_shm_svr::make(shm_sync, downstream_port);

            return std::make_pair(upstream_adapter, downstream_adapter);
        }
    }
};


} // namespace gr