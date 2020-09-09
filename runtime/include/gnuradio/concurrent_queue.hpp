#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <atomic>

#include <gnuradio/scheduler_message.hpp>

namespace gr {

template <typename T>
class concurrent_queue
{
public:
    bool push(const T& msg)
    {
        std::unique_lock<std::mutex> l(_mutex);
        _queue.push(msg);
        l.unlock();
        _cond.notify_all();

        return true;
    }
    bool pop(T& msg)
    {
        std::unique_lock<std::mutex> l(_mutex);
        _cond.wait(l, [this] { return !_queue.empty(); }); // TODO - replace with a waitfor

        msg = _queue.front();
        _queue.pop();
        return true;
    }
    bool empty()
    {
        std::unique_lock<std::mutex> l(_mutex);
        return _queue.empty();
        // l.unlock();
        // _cond.notify_all();
    }

private:
    std::queue<T> _queue;
    std::mutex _mutex;
    std::condition_variable _cond;
    std::atomic<bool> _ready;
};
} // namespace gr