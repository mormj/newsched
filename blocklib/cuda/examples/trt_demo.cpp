#include <chrono>
#include <iostream>
#include <thread>

#include <gnuradio/blocklib/blocks/head.hpp>
#include <gnuradio/blocklib/blocks/null_sink.hpp>
#include <gnuradio/blocklib/blocks/null_source.hpp>
#include <gnuradio/blocklib/blocks/vector_source.hpp>
#include <gnuradio/blocklib/cuda/infer.hpp>
#include <gnuradio/domain_adapter_shm.hpp>
#include <gnuradio/domain_adapter_direct.hpp>
#include <gnuradio/flowgraph.hpp>
#include <gnuradio/logging.hpp>
#include <gnuradio/schedulers/st/scheduler_st.hpp>

#include <gnuradio/cudabuffer_pinned.hpp>
#include <gnuradio/cudabuffer.hpp>

using namespace gr;

int main(int argc, char* argv[])
{
    auto logger = logging::get_logger("TRT_DEMO", "debug");

    // Basic test of the single threaded scheduler single instance
    if (1) {
        int samples = 1000000; // how many floats
        // std::vector<float> input_data(samples);
        // std::vector<float> expected_output(samples);
        // for (int i = 0; i < samples; i++) {
        //     input_data[i] = (float)i;
        //     expected_output[i] = input_data[i] * k;
        // }

        auto infer = cuda::infer::make("/share/gnuradio/benchmark-dnn/FCNSMALL_1.onnx",
                                       sizeof(float),
                                       cuda::memory_model_t::PINNED,
                                       1073741824,
                                       -1);
        std::vector<float> input_data(samples);
        for (auto i=0; i<samples; i++)
            input_data[i] = (float)(i%256);

        auto src = blocks::vector_source_f::make(input_data, false);
        // auto src = blocks::null_source::make(sizeof(float));
        auto snk = blocks::null_sink::make(sizeof(float));
        // auto head = blocks::head::make(sizeof(float), samples);

        auto fg(std::make_shared<flowgraph>());

        fg->connect(src, 0, infer, 0, cuda_buffer_pinned::make, cuda_buffer_pinned_properties::make());
        fg->connect(
            infer, 0, snk, 0, cuda_buffer_pinned::make, cuda_buffer_pinned_properties::make());

        if (0) {
            std::shared_ptr<schedulers::scheduler_st> sched1(
                new schedulers::scheduler_st("sched1", 32768));
            std::shared_ptr<schedulers::scheduler_st> sched2(
                new schedulers::scheduler_st("sched2", 32768));

            sched2->set_default_buffer_factory(cuda_buffer_pinned::make);
            // sched2->set_default_buffer_factory(cuda_buffer::make, cuda_buffer_properties::make(cuda_buffer_type::D2D));

            fg->add_scheduler(sched1);
            fg->add_scheduler(sched2);

            auto da_conf1 = domain_adapter_direct_conf::make(buffer_preference_t::UPSTREAM);
            auto da_conf2 =
                domain_adapter_direct_conf::make(buffer_preference_t::DOWNSTREAM);

            // domain_conf_vec dconf{ domain_conf(sched1, { src, head, snk }, da_conf1),
            //                        domain_conf(sched2, { infer }, da_conf2) };
            domain_conf_vec dconf{ domain_conf(sched1, { src, snk }, da_conf1),
                                   domain_conf(sched2, { infer }, da_conf2) };

            fg->partition(dconf);
        } else {
            std::shared_ptr<schedulers::scheduler_st> sched1(
                new schedulers::scheduler_st("sched1", 32768));
            fg->set_scheduler(sched1);
            fg->validate();
        }

        gr_log_debug(logger, "infer {}, src {}, snk, {}", infer->id(), src->id(), snk->id());

        auto t1 = std::chrono::steady_clock::now();
        fg->start();
        fg->wait();
        auto t2 = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> fp_ms = t2 - t1;
        std::cout << "Domain Adapter flowgraph took: " << fp_ms.count() << std::endl;
        std::cout << (1.0 / 1000.0) * (float)samples / fp_ms.count() << " MSamps/sec"
                  << std::endl;
    }
}