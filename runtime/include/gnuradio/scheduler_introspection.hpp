#include <stdint.h>

/**
 * @brief Scheduler Introspection class
 *
 * For the specified thread, keep track of statistics for scheduler events to track
 * scheduler performance
 *
 */
class scheduler_introspection
{
private:
    int64_t num_notify_input;
    int64_t num_notify_output;
    int64_t num_notify_all;
    int64_t num_work_notification;
    int64_t num_do_no_work;
    int64_t num_notify_upstream;
    int64_t num_notify_downstream;
    int64_t num_no_notify;

    // statistics for work functions per block 
    // int avg_
public:
    scheduler_introspection(/* args */);
    ~scheduler_introspection();
};
