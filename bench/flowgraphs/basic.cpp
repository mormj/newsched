#include <chrono>
#include <iostream>
#include <thread>

#include <gnuradio/blocklib/blocks/multiply_const.hpp>
#include <gnuradio/blocklib/blocks/null_source.hpp>
#include <gnuradio/blocklib/blocks/head.hpp>
#include <gnuradio/blocklib/blocks/vector_sink.hpp>
#include <gnuradio/blocklib/blocks/vector_source.hpp>
#include <gnuradio/domain_adapter_shm.hpp>
#include <gnuradio/flowgraph.hpp>
#include <gnuradio/schedulers/simplestream/scheduler_simplestream.hpp>

using namespace gr;

int main(int argc, char* argv[])
{
    float k = 1.0;
    int nsamps = 50000000;
    std::vector<float> input_data(nsamps);
    std::vector<float> expected_output(nsamps);
    for (int i = 0; i < nsamps; i++) {
        input_data[i] = i;
        expected_output[i] = i * k;
    }

    {
        // auto src = blocks::vector_source_f::make(input_data, false);
        auto src = blocks::null_source::make(sizeof(float));
        auto head = blocks::head::make(sizeof(float), nsamps);
        auto mult = blocks::multiply_const_ff::make(k);
        auto snk = blocks::vector_sink_f::make();

        flowgraph_sptr fg(new flowgraph());
        fg->connect(src, 0, head, 0);
        fg->connect(head, 0, mult, 0);
        fg->connect(mult, 0, snk, 0);

        std::shared_ptr<schedulers::scheduler_simplestream> sched1(
            new schedulers::scheduler_simplestream("sched1"));

        fg->add_scheduler(sched1);

        fg->validate();

        auto t1 = std::chrono::steady_clock::now();
        fg->start();
        fg->wait();

        auto t2 = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> fp_ms = t2 - t1;
        std::cout << "non-Domain Adapter flowgraph took: " << fp_ms.count() << std::endl;

        auto vec = snk->data();
        std::cout << (vec == expected_output) << std::endl;

        std::cout << std::endl;
    }
}
