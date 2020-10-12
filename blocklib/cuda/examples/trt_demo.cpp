#include <chrono>
#include <iostream>
#include <thread>

#include <gnuradio/blocklib/blocks/head.hpp>
#include <gnuradio/blocklib/blocks/null_sink.hpp>
#include <gnuradio/blocklib/blocks/null_source.hpp>
#include <gnuradio/blocklib/cuda/infer.hpp>
#include <gnuradio/domain_adapter_shm.hpp>
#include <gnuradio/flowgraph.hpp>
#include <gnuradio/logging.hpp>
#include <gnuradio/schedulers/st/scheduler_st.hpp>

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

        auto infer = cuda::infer::make("/share/gnuradio/benchmark-dnn/FCNSMALL_1.onnx", sizeof(float), cuda::memory_model_t::PINNED, 1073741824, -1);
        auto src = blocks::null_source::make(sizeof(float));
        auto snk = blocks::null_sink::make(sizeof(float));
        auto head = blocks::head::make(sizeof(float), samples);

        auto fg(std::make_shared<flowgraph>());

        fg->connect(src, 0, infer, 0);
        fg->connect(infer, 0, head, 0);
        fg->connect(head, 0, snk, 0);

        std::shared_ptr<schedulers::scheduler_st> sched(
            new schedulers::scheduler_st("sched1", 256));
        fg->set_scheduler(sched);

        fg->validate();

        auto t1 = std::chrono::steady_clock::now();
        fg->start();
        fg->wait();
        auto t2 = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> fp_ms = t2 - t1;
        std::cout << "Domain Adapter flowgraph took: " << fp_ms.count() << std::endl;
        std::cout << (1.0/1000.0) * (float)samples / fp_ms.count() << " MSamps/sec" << std::endl;

    }

}
