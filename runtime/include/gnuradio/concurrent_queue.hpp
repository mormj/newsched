#pragma once

#include <condition_variable>
#include <atomic>
#include <chrono>
#include <deque>
#include <iostream>
#include <mutex>
#include <vector>

#include <gnuradio/scheduler_message.hpp>

using namespace std::chrono_literals;
namespace gr {

#if 1
template <typename T>
class concurrent_queue
{
public:
    bool push(const T& msg)
    {
        // std::cout << "**push" << std::endl;
        std::unique_lock<std::mutex> l(_mutex);
        _queue.push_back(msg);
        l.unlock();
        _cond.notify_all();

        return true;
    }
    bool pop(T& msg)
    {
        std::unique_lock<std::mutex> l(_mutex);

        if (_num_pop < 5)
        {
        _cond.wait(l,
                   [this] { return !_queue.empty(); }); // TODO - replace with a waitfor
        // if (_cond.wait_for(l, 250ms, [this] { return !_queue.empty(); })) {
            msg = _queue.front();
            _queue.pop_front();
            _num_pop++;
            return true;
        // } else {
        //     return false;
        // }
            // _num_pop++;
        }

        if (_queue.size() > 0)
        {
            msg = _queue.front();
            _queue.pop_front();
            return true;
        }
        else{
            return false;
        }

    }
    void clear()
    {
        std::unique_lock<std::mutex> l(_mutex);
        _queue.clear();
        // l.unlock();
        // _cond.notify_all();
    }

    size_t size()
    {
        std::unique_lock<std::mutex> l(_mutex);
        return _queue.size();
    }

private:
    std::deque<T> _queue;
    std::mutex _mutex;
    std::condition_variable _cond;
    std::atomic<bool> _ready;

    int _num_pop = 0;
};

#else
template <typename T>
class concurrent_queue
{
public:
    static const uint16_t MAX_QUEUE_SIZE = 1024;
    concurrent_queue() { _vec.resize(MAX_QUEUE_SIZE); }
    bool empty() { return _rd_idx == _wr_idx; }
    bool push(const T& msg)
    {
        // std::cout << "**push" << std::endl;
        std::unique_lock<std::mutex> l(_mutex);
        // _queue.push_back(msg);
        _vec[_wr_idx++] = msg;
        if (_wr_idx >= MAX_QUEUE_SIZE) {
            _wr_idx = 0;
        }
        l.unlock();
        _cond.notify_one();

        return true;
    }
    bool pop(T& msg)
    {
        std::unique_lock<std::mutex> l(_mutex);
        _cond.wait(l, [this] { return !empty(); }); // TODO - replace with a waitfor
        // msg = _queue.front();
        // _queue.pop_front();
        msg = _vec[_rd_idx++];
        if (_rd_idx >= MAX_QUEUE_SIZE) {
            _rd_idx = 0;
        }
        return true;
    }
    void clear()
    {
        std::unique_lock<std::mutex> l(_mutex);
        // _queue.clear();
        // l.unlock();
        // _cond.notify_all();
    }

private:
    std::vector<T> _vec;
    // std::deque<T> _queue;
    uint16_t _rd_idx = 0;
    uint16_t _wr_idx = 0;

    std::mutex _mutex;
    std::condition_variable _cond;
    std::atomic<bool> _ready;
};
#endif

} // namespace gr