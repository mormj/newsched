#include <chrono>
#include <iostream>
#include <thread>

#include <gnuradio/blocklib/blocks/head.hpp>
#include <gnuradio/blocklib/blocks/multiply_const.hpp>
#include <gnuradio/blocklib/blocks/vector_sink.hpp>
#include <gnuradio/blocklib/blocks/vector_source.hpp>
#include <gnuradio/domain_adapter_shm.hpp>
#include <gnuradio/flowgraph.hpp>
#include <gnuradio/schedulers/default/inner.hpp>
#include <gnuradio/logging.hpp>

using namespace gr;

int main(int argc, char* argv[])
{
    auto logger = logging::get_logger("TEST_INNER", "debug");

    if (0)
    {
        int samples = 10000;
        float k = 100.0;
        std::vector<float> input_data(samples);
        std::vector<float> expected_output(samples);
        for(int i=0; i<samples; i++)
        {
            input_data[i] = (float)i;
            expected_output[i] = input_data[i] * k;
        }
        auto src = blocks::vector_source_f::make(
            input_data, true);
        auto mult = blocks::multiply_const_ff::make(k);
        auto head = blocks::head::make(sizeof(float), samples);
        auto snk = blocks::vector_sink_f::make();

        auto fg(std::make_shared<flowgraph>());
        fg->connect(src, 0, mult, 0);
        fg->connect(mult, 0, head, 0);
        fg->connect(head, 0, snk, 0);

        std::shared_ptr<schedulers::scheduler_default_inner> sched(
            new schedulers::scheduler_default_inner());
        fg->set_scheduler(sched);

        fg->validate();
        fg->start();
        fg->wait();

        auto snk_data = snk->data();
        gr_log_debug(logger, "valid output: {}, {}", snk_data == expected_output, snk_data.size());
    }


    {
        int samples = 100000;
        float k = 100.0;
        std::vector<float> input_data(samples);
        std::vector<float> expected_output(samples);
        for(int i=0; i<samples; i++)
        {
            input_data[i] = (float)i;
            expected_output[i] = input_data[i] * k * k;
        }
        auto src = blocks::vector_source_f::make(
            input_data, true);
        auto mult1 = blocks::multiply_const_ff::make(k);
        auto mult2 = blocks::multiply_const_ff::make(k);
        auto head = blocks::head::make(sizeof(float), samples);
        auto snk = blocks::vector_sink_f::make();

        auto fg(std::make_shared<flowgraph>());
        fg->connect(src, 0, mult1, 0);
        fg->connect(mult1, 0, mult2, 0);
        fg->connect(mult2, 0, head, 0);
        fg->connect(head, 0, snk, 0);

        auto sched1 = std::make_shared<schedulers::scheduler_default_inner>("sched1");
        auto sched2 = std::make_shared<schedulers::scheduler_default_inner>("sched2");

        fg->add_scheduler(sched1);
        fg->add_scheduler(sched2);

        auto da_conf = domain_adapter_shm_conf::make(buffer_preference_t::UPSTREAM);

        domain_conf_vec dconf{ domain_conf(sched1, { src, mult1 }, da_conf),
                               domain_conf(sched2, { mult2, head, snk }, da_conf) };

        fg->partition(dconf);


        // fg->validate();
        fg->start();
        fg->wait();

        auto snk_data = snk->data();
        gr_log_info(logger, "valid output: {}, {}", snk_data == expected_output, snk_data.size());
    }
}
