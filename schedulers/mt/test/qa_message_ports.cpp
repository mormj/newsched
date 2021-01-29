#include <gtest/gtest.h>

#include <chrono>
#include <iostream>
#include <thread>

#include <gnuradio/blocklib/blocks/msg_forward.hpp>
#include <gnuradio/flowgraph.hpp>
#include <gnuradio/schedulers/mt/scheduler_mt.hpp>

using namespace gr;

TEST(SchedulerMTMessagePassing, Forward)
{
    std::vector<float> input_data{ 1.0, 2.0, 3.0, 4.0, 5.0 };

    auto blk1 = blocks::msg_forward::make();
    auto blk2 = blocks::msg_forward::make();
    auto blk3 = blocks::msg_forward::make();

    flowgraph_sptr fg(new flowgraph());
    fg->connect(blk1, 0, blk2, 0);
    fg->connect(blk2, 0, blk3, 0);

    std::shared_ptr<schedulers::scheduler_mt> sched(new schedulers::scheduler_mt());
    fg->set_scheduler(sched);

    fg->validate();

    auto src_port = blk1->get_port("input");


    fg->start();
    fg->wait();

}
