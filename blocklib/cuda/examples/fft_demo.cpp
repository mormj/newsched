#include <chrono>
#include <iostream>
#include <thread>

#include <gnuradio/blocklib/blocks/head.hpp>
#include <gnuradio/blocklib/blocks/null_sink.hpp>
#include <gnuradio/blocklib/blocks/null_source.hpp>
#include <gnuradio/blocklib/blocks/vector_sink.hpp>
#include <gnuradio/blocklib/blocks/vector_source.hpp>
#include <gnuradio/blocklib/cuda/fft.hpp>
#include <gnuradio/domain_adapter_direct.hpp>
#include <gnuradio/domain_adapter_shm.hpp>
#include <gnuradio/flowgraph.hpp>
#include <gnuradio/logging.hpp>
#include <gnuradio/schedulers/st/scheduler_st.hpp>

#include <gnuradio/simplebuffer.hpp>
#include <gnuradio/cudabuffer.hpp>
#include <gnuradio/cudabuffer_pinned.hpp>

using namespace gr;

int main(int argc, char* argv[])
{
    auto logger = logging::get_logger("FFT_DEMO", "debug");

    int batch_size = 1;
    int fft_size = 4;
    int samples = 1000000; // how many floats
    int actual_samples = (fft_size * batch_size) * (samples / (fft_size * batch_size));
//         std::vector<float> bh_window{
// #include "window_blackmanharris_32.h"
//         };
    // Basic test of the single threaded scheduler single instance
    if (0) {

        auto fft = cuda::fft::make(fft_size, true, false, batch_size);

        std::vector<gr_complex> input_data(fft_size * (samples / fft_size));
        for (auto i = 0; i < fft_size * (samples / fft_size); i++)
            input_data[i] = gr_complex(i % 256, 255 - (i % 256));

        // auto src = blocks::vector_source_c::make(input_data, false, fft_size);
        auto src = blocks::null_source::make(sizeof(gr_complex) * fft_size);
        auto snk = blocks::null_sink::make(sizeof(gr_complex) * fft_size);
        // auto snk = blocks::vector_sink_c::make(fft_size);
        auto head = blocks::head::make(sizeof(gr_complex) * fft_size, samples / fft_size);

        auto fg(std::make_shared<flowgraph>());

        fg->connect(src, 0, head, 0);
        fg->connect(head,
        // fg->connect(src,
                    0,
                    fft,
                    0,
                    // CUDA_BUFFER_PINNED_ARGS);
                    CUDA_BUFFER_ARGS_H2D);
        fg->connect(fft,
                    0,
                    snk,
                    0,
                    // CUDA_BUFFER_PINNED_ARGS);
                    CUDA_BUFFER_ARGS_D2H);

        // fg->connect(src, 0, head, 0);
        // fg->connect(head,
        //             // fg->connect(src,
        //             0,
        //             fft,
        //             0,
        //             CUDA_BUFFER_ARGS_H2D);
        // fg->connect(fft,
        //             0,
        //             snk,
        //             0,
        //             CUDA_BUFFER_ARGS_D2H);


        std::shared_ptr<schedulers::scheduler_st> sched1(
            new schedulers::scheduler_st("sched1", 32768));
        fg->set_scheduler(sched1);
        fg->validate();

        gr_log_debug(logger, "fft {}, src {}, snk, {}", fft->id(), src->id(), snk->id());

        auto t1 = std::chrono::steady_clock::now();
        fg->start();
        fg->wait();

#if 0
        auto d = snk->data();
        std::cout << "data.size() = " << d.size() << std::endl;
        for (int i=0; i<100; i++)
        {
            std::cout << real(d[i]) << "+" << imag(d[i]) << ",";
        }
        std::cout << std::endl;
#endif
        auto t2 = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> fp_ms = t2 - t1;
        std::cout << "fft_demo took: " << fp_ms.count() << std::endl;
        std::cout << (1.0 / 1000.0) * (float)actual_samples / fp_ms.count()
                  << " MSamps/sec" << std::endl;
    }

    else if (1) {

        std::vector<cuda::fft::sptr> fft_blks(4);
        for (int i = 0; i < 4; i++) {
            fft_blks[i] = cuda::fft::make(fft_size, true, false, batch_size);
        }

        std::vector<gr_complex> input_data(samples);
        for (auto i = 0; i < samples; i++)
            input_data[i] = gr_complex(i % 256, 256 - i % 256);

        // auto src = blocks::vector_source_c::make(input_data, false, fft_size);
        auto src = blocks::null_source::make(sizeof(gr_complex) * fft_size);
        auto snk = blocks::null_sink::make(sizeof(gr_complex) * fft_size);
        // auto snk = blocks::vector_sink_c::make(fft_size);
        auto head = blocks::head::make(sizeof(gr_complex) * fft_size, samples / fft_size);

        auto fg(std::make_shared<flowgraph>());

        fg->connect(src, 0, head, 0, SIMPLE_BUFFER_ARGS);
        fg->connect(head, 0, fft_blks[0], 0, CUDA_BUFFER_ARGS_H2D);
        fg->connect(fft_blks[0], 0, fft_blks[1], 0, CUDA_BUFFER_ARGS_D2D);
        fg->connect(fft_blks[1], 0, fft_blks[2], 0, CUDA_BUFFER_ARGS_D2D);
        fg->connect(fft_blks[2], 0, fft_blks[3], 0, CUDA_BUFFER_ARGS_D2D);
        fg->connect(fft_blks[3], 0, snk, 0, CUDA_BUFFER_ARGS_D2H);

        std::shared_ptr<schedulers::scheduler_st> sched1(
            new schedulers::scheduler_st("sched1", 32768));
        sched1->set_default_buffer_factory(CUDA_BUFFER_ARGS_D2D);
        std::shared_ptr<schedulers::scheduler_st> sched2(
            new schedulers::scheduler_st("sched2", 32768));
        sched2->set_default_buffer_factory(CUDA_BUFFER_ARGS_D2D);
        std::shared_ptr<schedulers::scheduler_st> sched3(
            new schedulers::scheduler_st("sched3", 32768));
        sched3->set_default_buffer_factory(CUDA_BUFFER_ARGS_D2D);
        std::shared_ptr<schedulers::scheduler_st> sched4(
            new schedulers::scheduler_st("sched4", 32768));
        sched4->set_default_buffer_factory(CUDA_BUFFER_ARGS_D2D);

        fg->add_scheduler(sched1);
        fg->add_scheduler(sched2);
        fg->add_scheduler(sched3);
        fg->add_scheduler(sched4);

        auto da_conf1 = domain_adapter_direct_conf::make(buffer_preference_t::DOWNSTREAM);
        auto da_conf2 = domain_adapter_direct_conf::make(buffer_preference_t::DOWNSTREAM);

        // domain_conf_vec dconf{ domain_conf(sched1, { src, head, snk }, da_conf1),
        //                        domain_conf(sched2, { infer }, da_conf2) };
        domain_conf_vec dconf{ domain_conf(sched1, { src, head, fft_blks[0] }, da_conf1),
                               domain_conf(sched2, { fft_blks[1] }, da_conf2),
                               domain_conf(sched3, { fft_blks[2] }, da_conf2),
                               domain_conf(sched4, { fft_blks[3], snk }, da_conf2) };

        fg->partition(dconf);

        // gr_log_debug(logger, "fft {}, src {}, snk, {}", fft->id(), src->id(),
        // snk->id());

        auto t1 = std::chrono::steady_clock::now();
        fg->start();
        fg->wait();
        auto t2 = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> fp_ms = t2 - t1;
        std::cout << "Domain Adapter flowgraph took: " << fp_ms.count() << std::endl;
        std::cout << (1.0 / 1000.0) * (float)actual_samples / fp_ms.count()
                  << " MSamps/sec" << std::endl;
    }
}
