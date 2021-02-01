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
    fg->connect(blk1, "out", blk2, "in");
    fg->connect(blk2, "out", blk3, "in");

    std::shared_ptr<schedulers::scheduler_mt> sched(new schedulers::scheduler_mt());
    fg->set_scheduler(sched);

    fg->validate();

    auto src_port = blk1->get_message_port("out");
    src_port->post("message");

    fg->start();
    fg->wait();

}
