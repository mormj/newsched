#include <chrono>
#include <iostream>
#include <thread>

#include <gnuradio/blocklib/blocks/dummy.hpp>
#include <gnuradio/blocklib/blocks/throttle.hpp>
#include <gnuradio/blocklib/blocks/vector_sink.hpp>
#include <gnuradio/blocklib/blocks/vector_source.hpp>
#include <gnuradio/flowgraph.hpp>
#include <gnuradio/schedulers/simplestream/scheduler_simplestream.hpp>

using namespace gr;

int main(int argc, char* argv[])
{
    auto src = blocks::vector_source_f::make(
        std::vector<float>{ 1.0, 2.0, 3.0, 4.0, 5.0 }, true);
    auto throttle = blocks::throttle::make(sizeof(float), 100);
    auto mult1 = blocks::multiply_const_ff::make(100.0);
    auto mult2 = blocks::multiply_const_ff::make(200.0);
    auto snk = blocks::vector_sink_f::make();

    // blocks::vector_sink_f snk();

    flowgraph_sptr fg(new flowgraph());
    fg->connect(src->base(), 0, throttle->base(), 0);
    fg->connect(throttle->base(), 0, mult1->base(), 0);
    fg->connect(mult1->base(), 0, mult2->base(), 0);
    fg->connect(mult2->base(), 0, snk->base(), 0);

    std::shared_ptr<schedulers::scheduler_simplestream> sched1(
        new schedulers::scheduler_simplestream());
    std::shared_ptr<schedulers::scheduler_simplestream> sched2(
        new schedulers::scheduler_simplestream());

    fg->add_scheduler(sched1->base());
    fg->add_scheduler(sched2->base());

    std::vector<std::tuple<block_sptr, scheduler_sptr>> partitions
    {
        { src, sched1 }, { throttle, sched1 }, { mult1, sched1 }, { mult2, sched2 },
        {
            snk, sched2
        }
    }

    fg->partion(partitions)
    fg->validate();

    // what happens when k is set here???

    fg->start();

    auto start = std::chrono::steady_clock::now();

    float a = 1.0;
    float b = 100.0;
    while (true) {
        std::cout << "query a is " << dummy->a() << std::endl;
        std::cout << "query b is " << dummy->b() << std::endl;

        dummy->set_a(a);
        dummy->set_b(b);

        if (std::chrono::steady_clock::now() - start > std::chrono::seconds(200))
            break;

        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        a += 1.0;
        b += 1.0;

        for (const auto& d : snk1->data())
            std::cout << d << ' ';
        std::cout << std::endl;

        for (const auto& d : snk2->data())
            std::cout << d << ' ';
        std::cout << std::endl;
    }

    fg->stop();
    fg->wait();

    // DOMAIN??

    // now look at the data
}