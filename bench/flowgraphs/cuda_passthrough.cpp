#include <chrono>
#include <iostream>
#include <thread>

#include <gnuradio/blocklib/blocks/head.hpp>
#include <gnuradio/blocklib/blocks/null_sink.hpp>
#include <gnuradio/blocklib/blocks/null_source.hpp>
#include <gnuradio/blocklib/blocks/vector_sink.hpp>
#include <gnuradio/blocklib/blocks/vector_source.hpp>
#include <gnuradio/blocklib/cuda/passthrough.hpp>
#include <gnuradio/domain_adapter_direct.hpp>
#include <gnuradio/domain_adapter_shm.hpp>
#include <gnuradio/flowgraph.hpp>
#include <gnuradio/logging.hpp>
#include <gnuradio/realtime.hpp>
#include <gnuradio/schedulers/st/scheduler_st.hpp>

#include <gnuradio/cudabuffer.hpp>
#include <gnuradio/cudabuffer_pinned.hpp>
#include <gnuradio/simplebuffer.hpp>
#include <iostream>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

using namespace gr;

int main(int argc, char* argv[])
{
    int run;
    uint64_t samples;
    int nsched;
    int mem_model;
    int batch_size;
    int nblocks;
    bool rt_prio = false;
    bool machine_readable = false;

    po::options_description desc("CUDA FFT Benchmarking Flowgraph");
    desc.add_options()("help,h", "display help")(
        "run,R", po::value<int>(&run)->default_value(0), "Run Number")(
        "samples,N",
        po::value<uint64_t>(&samples)->default_value(15000000),
        "Number of samples")(
        "nblocks,b", po::value<int>(&nblocks)->default_value(4), "Num FFT Blocks")(
        "memmodel,m", po::value<int>(&mem_model)->default_value(0), "Memory Model")(
        "batchsize,s", po::value<int>(&batch_size)->default_value(1), "Batch Size")(
        "machine_readable,m", "Machine-readable Output")("rt_prio,t",
                                                         "Enable Real-time priority");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }

    if (vm.count("machine_readable")) {
        machine_readable = true;
    }

    if (vm.count("rt_prio")) {
        rt_prio = true;
    }

    if (rt_prio && gr::enable_realtime_scheduling() != RT_OK) {
        std::cout << "Error: failed to enable real-time scheduling." << std::endl;
    }

    auto da_conf = domain_adapter_direct_conf::make(buffer_preference_t::DOWNSTREAM);


    std::vector<cuda::passthrough::sptr> passthrough_blks(nblocks);
    for (int i = 0; i < nblocks; i++) {
        passthrough_blks[i] = cuda::passthrough::make(batch_size);
    }

    std::vector<gr_complex> input_data(samples);
    for (auto i = 0; i < samples; i++)
        input_data[i] = gr_complex(i % 256, 256 - i % 256);

    auto src = blocks::null_source::make(sizeof(gr_complex) * batch_size);
    auto snk = blocks::null_sink::make(sizeof(gr_complex) * batch_size);
    auto head = blocks::head::make(sizeof(gr_complex) * batch_size, samples / batch_size);

    auto fg(std::make_shared<flowgraph>());

    fg->connect(src, 0, head, 0, SIMPLE_BUFFER_ARGS);

    std::vector<std::shared_ptr<schedulers::scheduler_st>> scheds(nblocks+3);
    if (mem_model == 0) {
        fg->connect(head, 0, passthrough_blks[0], 0, CUDA_BUFFER_ARGS_H2D);
        for (int i = 0; i < nblocks - 1; i++) {
            fg->connect(passthrough_blks[i], 0, passthrough_blks[i + 1], 0, CUDA_BUFFER_ARGS_D2D);
        }
        fg->connect(passthrough_blks[nblocks - 1], 0, snk, 0, CUDA_BUFFER_ARGS_D2H);

        for (int i = 0; i < nblocks; i++) {
            scheds[i] = std::make_shared<schedulers::scheduler_st>("sched1", 32768);
            scheds[i]->set_default_buffer_factory(CUDA_BUFFER_ARGS_D2D);
            fg->add_scheduler(scheds[i]);
        }

        for (int i=0; i<3; i++)
        {
            scheds[nblocks+i] = std::make_shared<schedulers::scheduler_st>("sched1_blk", 32768);
            scheds[nblocks+i]->set_default_buffer_factory(SIMPLE_BUFFER_ARGS);
            fg->add_scheduler(scheds[nblocks+i]);
        }

    } else {
        fg->connect(head, 0, passthrough_blks[0], 0, CUDA_BUFFER_PINNED_ARGS);
        for (int i = 0; i < nblocks - 1; i++) {
            fg->connect(passthrough_blks[i], 0, passthrough_blks[i + 1], 0, CUDA_BUFFER_PINNED_ARGS);
        }
        fg->connect(passthrough_blks[nblocks - 1], 0, snk, 0, CUDA_BUFFER_PINNED_ARGS);

        for (int i = 0; i < nblocks; i++) {
            scheds[i] = std::make_shared<schedulers::scheduler_st>("sched1", 32768);
            scheds[i]->set_default_buffer_factory(CUDA_BUFFER_PINNED_ARGS);
            fg->add_scheduler(scheds[i]);
        }

        for (int i=0; i<3; i++)
        {
            scheds[nblocks+i] = std::make_shared<schedulers::scheduler_st>("sched1_blk", 32768);
            scheds[nblocks+i]->set_default_buffer_factory(SIMPLE_BUFFER_ARGS);
            fg->add_scheduler(scheds[nblocks+i]);
        }
    }


    domain_conf_vec dconf;


    // if (nblocks == 1) {
    //     dconf.push_back(domain_conf(scheds[0], { src, head, passthrough_blks[0], snk }, da_conf));
    // } else {
    //     dconf.push_back(domain_conf(scheds[0], { src, head, passthrough_blks[0] }, da_conf));
    //     for (int i = 1; i < nblocks - 1; i++) {
    //         dconf.push_back(domain_conf(scheds[i], { passthrough_blks[i] }, da_conf));
    //     }
    //     dconf.push_back(
    //         domain_conf(scheds[nblocks - 1], { passthrough_blks[nblocks - 1], snk }, da_conf));
    // }

    for (int i=0; i<nblocks; i++)
    {
        dconf.push_back(domain_conf(scheds[i], { passthrough_blks[0] }, da_conf));
    }
    dconf.push_back(domain_conf(scheds[nblocks+0], { src }, da_conf));
    dconf.push_back(domain_conf(scheds[nblocks+1], { head }, da_conf));
    dconf.push_back(domain_conf(scheds[nblocks+2], { snk }, da_conf));
    
    fg->partition(dconf);

    if (rt_prio && gr::enable_realtime_scheduling() != gr::rt_status_t::RT_OK)
        std::cout << "Unable to enable realtime scheduling " << std::endl;

    auto t1 = std::chrono::steady_clock::now();
    fg->start();
    fg->wait();

    auto t2 = std::chrono::steady_clock::now();
    auto time =
        std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count() / 1e9;

    std::cout << "[PROFILE]" << time << "[PROFILE]" << std::endl;
}