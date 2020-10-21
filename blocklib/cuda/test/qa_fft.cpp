// #define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
// #include <doctest.h>
#define CATCH_CONFIG_MAIN
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

#include <gnuradio/blocklib/cuda/fft.hpp>

using namespace gr;


TEST_CASE("block outputs one output to 2 input blocks")
{
    int batch_size = 1;
    int fft_size = 256;
    int samples = 512; // how many floats
    int actual_samples = (fft_size * batch_size) * (samples / (fft_size * batch_size));

    auto fft = cuda::fft::make(fft_size, true, false, batch_size);
    auto fft2 = cuda::fft::make(fft_size, false, false, batch_size);

    std::vector<gr_complex> input_data(actual_samples);
    for (auto i = 0; i < actual_samples; i++)
        input_data[i] = gr_complex(i % 256, 255 - (i % 256));

    auto src = blocks::vector_source_c::make(input_data, false, fft_size);
    auto snk = blocks::vector_sink_c::make(fft_size);

    auto fg(std::make_shared<flowgraph>());

    fg->connect(src,
                0,
                fft,
                0,
                // CUDA_BUFFER_PINNED_ARGS);
                CUDA_BUFFER_ARGS_H2D);
    fg->connect(fft,
                0,
                fft2,
                0,
                // CUDA_BUFFER_PINNED_ARGS);
                CUDA_BUFFER_ARGS_D2D);
    fg->connect(fft2,
                0,
                snk,
                0,
                // CUDA_BUFFER_PINNED_ARGS);
                CUDA_BUFFER_ARGS_D2H);

    std::shared_ptr<schedulers::scheduler_st> sched1(
        new schedulers::scheduler_st("sched1", 32768));
    fg->set_scheduler(sched1);
    fg->validate();

    fg->start();
    fg->wait();

    auto d = snk->data();


    REQUIRE_THAT(d, Catch::Equals(input_data));
 
}