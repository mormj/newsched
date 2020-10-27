// #define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
// #include <doctest.h>
// #define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <chrono>
#include <iostream>
#include <thread>

#include <gnuradio/blocklib/blocks/fanout.hpp>
#include <gnuradio/blocklib/blocks/multiply_const.hpp>
#include <gnuradio/blocklib/blocks/throttle.hpp>
#include <gnuradio/blocklib/blocks/vector_sink.hpp>
#include <gnuradio/blocklib/blocks/vector_source.hpp>
#include <gnuradio/domain_adapter_shm.hpp>
#include <gnuradio/flowgraph.hpp>
#include <gnuradio/schedulers/st/scheduler_st.hpp>

#include <gnuradio/cudabuffer.hpp>
#include <gnuradio/cudabuffer_pinned.hpp>

#include <gnuradio/blocklib/cuda/passthrough.hpp>

using namespace gr;


TEST_CASE("Test passthrough CUDA Block")
{
    for (int k=1; k<6; k++)
    {
    int batch_size = 1024*k;
    int samples = 1024*1024*k; // how many floats

    auto blk = cuda::passthrough::make(batch_size);

    std::vector<gr_complex> input_data(samples);
    for (auto i = 0; i < samples; i++)
        input_data[i] = gr_complex(i % 256, 255 - (i % 256));
    
    auto expected_output = input_data;

    auto src = blocks::vector_source_c::make(input_data, false, batch_size);
    auto snk = blocks::vector_sink_c::make(batch_size);

    auto fg(std::make_shared<flowgraph>());

    fg->connect(src,
                0,
                blk,
                0,
                CUDA_BUFFER_PINNED_ARGS);
                // CUDA_BUFFER_ARGS_H2D);
    fg->connect(blk,
                0,
                snk,
                0,
                CUDA_BUFFER_PINNED_ARGS);
                // CUDA_BUFFER_ARGS_D2H);

    std::shared_ptr<schedulers::scheduler_st> sched1(
        new schedulers::scheduler_st("sched1", 32768));
    fg->set_scheduler(sched1);
    fg->validate();

    fg->start();
    fg->wait();

    auto d = snk->data();


    REQUIRE_THAT(d, Catch::Equals(expected_output));
    }
}