#pragma once

#include <gnuradio/block.hpp>
#include <gnuradio/buffer.hpp>
#include <gnuradio/executor.hpp>
#include <gnuradio/scheduler_introspection.hpp>

#include "buffer_management.hpp"

#include <map>

namespace gr {
namespace schedulers {

class graph_executor : public executor
{
private:
    std::vector<block_sptr> d_blocks;
    scheduler_introspection* _si;

    // Move to buffer management
    const int s_fixed_buf_size;
    static const int s_min_items_to_process = 1;
    const size_t s_max_buf_items; // = s_fixed_buf_size / 2;
    const size_t s_min_buf_items = 1;

    buffer_manager::sptr _bufman;

public:
    graph_executor(const std::string& name, scheduler_introspection* si = nullptr)
        : executor(name), _si(si), s_fixed_buf_size(32768), s_max_buf_items(32768 - 1){};
    ~graph_executor(){};

    void initialize(buffer_manager::sptr bufman, std::vector<block_sptr> blocks)
    {
        _bufman = bufman;
        d_blocks = blocks;
    }

    std::map<nodeid_t, executor_iteration_status>
    run_one_iteration(std::vector<block_sptr> blocks = std::vector<block_sptr>());

    int64_t pc_n_times_work_called = 0;
};

} // namespace schedulers
} // namespace gr