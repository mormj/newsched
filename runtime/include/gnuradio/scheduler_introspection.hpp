#pragma once

#include <stdint.h>
#include <iostream>

#include <gnuradio/logging.hpp>

namespace gr {
/**
 * @brief Scheduler Introspection class
 *
 * For the specified thread, keep track of statistics for scheduler events to track
 * scheduler performance
 *
 */
struct scheduler_introspection {

    int64_t num_work_called = 0;
    int64_t num_in_not_ready = 0;
    int64_t num_out_not_ready = 0;
    int64_t num_work_done = 0;
    int64_t num_work_ok = 0;
    int64_t num_work_insufficient_input = 0;
    int64_t num_notify_input = 0;
    int64_t num_notify_output = 0;
    int64_t num_notify_all = 0;
    int64_t num_work_notification = 0;
    int64_t num_do_no_work = 0;
    int64_t num_notify_upstream = 0;
    int64_t num_notify_downstream = 0;
    int64_t num_no_notify = 0;

    void print(logger_sptr logger)
    {
        gr_log_info(logger, "num_work_called: {}", num_work_called);
        gr_log_info(logger, "num_in_not_ready: {}", num_in_not_ready);
        gr_log_info(logger, "num_out_not_ready: {}", num_out_not_ready);
        gr_log_info(logger, "num_work_done: {}", num_work_done);
        gr_log_info(logger, "num_work_ok: {}", num_work_ok);
        gr_log_info(logger, "num_work_insufficient_input: {}", num_work_insufficient_input);        
        
        gr_log_info(logger, "num_work_notification: {}", num_work_notification);
        gr_log_info(logger, "num_notify_upstream: {}", num_notify_upstream);
        gr_log_info(logger, "num_notify_downstream: {}", num_notify_downstream);
        gr_log_info(logger, "num_notify_input: {}", num_notify_input);
        gr_log_info(logger, "num_notify_output: {}", num_notify_output);
        gr_log_info(logger, "num_notify_all: {}", num_notify_all);

    }
};
} // namespace gr

// INTROSPECT_INCREMENT(_si, notify_input);
#define INTROSPECT_INCREMENT(obj, var) obj.var++;


// INTROSPECT_record(_si, nproduced, n);
#define INTROSPECT_RECORD(obj, var, value) ()
