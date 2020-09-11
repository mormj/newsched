#pragma once
#include <string.h>
#include <atomic>
#include <thread>

#include <gnuradio/block.hpp>
#include <pmt/pmt.hpp>

namespace gr {
namespace blocks {
class msg_strobe : public block
{
public:
    enum params : uint32_t { id_msg, id_period_ms, num_params };

    typedef std::shared_ptr<msg_strobe> sptr;
    static sptr make(pmt::pmt_t msg, long period_ms)
    {
        auto ptr = std::make_shared<msg_strobe>(msg_strobe(msg, period_ms));

        ptr->add_param(param<pmt::pmt_t>::make(
            msg_strobe::params::id_msg, "msg", msg, &(ptr->_msg)));

        ptr->add_param(param<long>::make(msg_strobe::params::id_period_ms,
                                         "period_ms",
                                         period_ms,
                                         &(ptr->_period_ms)));

        ptr->add_port(message_port::make("strobe", port_direction_t::OUTPUT));

        return ptr;
    }

    msg_strobe(pmt::pmt_t msg, long period_ms)
        : block("msg_strobe"), _msg(msg), _period_ms(period_ms), _finished(false)
    {
    }

    bool start()
    {
        _finished = false;
        _thread = std::thread(std::bind(&msg_strobe::run, this));

        return block::start();
    }

    bool stop()
    {
        // Shut down the thread
        _finished = true;
        // _thread.interrupt();
        _thread.join();

        return block::stop();
    }


private:
    std::thread _thread;
    // std::atomic<bool> _finished;
    bool _finished; // FIXME - atomic bool deletes copy constructor - but needed here.
                    // figure out mechanics to get this atomic
    long _period_ms;
    pmt::pmt_t _msg;
    std::string _port = "strobe";

    void run()
    {
        while (!_finished) {
            std::this_thread::sleep_for(std::chrono::milliseconds(_period_ms));
            if (_finished) {
                return;
            }

            message_port_pub(_port, _msg);
        }
    }
};
} // namespace blocks
} // namespace gr