#include <chrono>
#include <iostream>
#include <thread>

#include <gnuradio/blocklib/blocks/multiply_const.hpp>
#include <gnuradio/blocklib/blocks/vector_sink.hpp>
#include <gnuradio/blocklib/blocks/vector_source.hpp>
#include <gnuradio/blocklib/blocks/head.hpp>
#include <gnuradio/domain_adapter_shm.hpp>
#include <gnuradio/flowgraph.hpp>
#include <gnuradio/schedulers/default/inner.hpp>

using namespace gr;

int main(int argc, char* argv[])
{
    int samples = 10000;
    auto src = blocks::vector_source_f::make(
        std::vector<float>{ 1.0, 2.0, 3.0, 4.0, 5.0 }, true);
    auto mult = blocks::multiply_const_ff::make(100.0);
    auto head = blocks::head::make(sizeof(float), samples);
    auto snk = blocks::vector_sink_f::make();

    flowgraph_sptr fg(new flowgraph());
    fg->connect(src, 0, mult, 0);
    fg->connect(mult, 0, head, 0);
    fg->connect(head, 0, snk, 0);

    std::shared_ptr<schedulers::scheduler_default_inner> sched(
        new schedulers::scheduler_default_inner());
    fg->set_scheduler(sched);

    fg->validate();
    fg->start();
    fg->wait();

    // for (const auto& d : snk1->data())
    //     std::cout << d << ' ';
    // std::cout << std::endl;

    // for (const auto& d : snk2->data())
    //     std::cout << d << ' ';
    // std::cout << std::endl;
  
}
