#pragma once

#include <gnuradio/block.hpp>

namespace gr {
namespace blocks {

class msg_forward : public block
{
public:
    typedef std::shared_ptr<msg_forward> sptr;
    static sptr make()
    {
        return std::make_shared<msg_forward>();
    }

    msg_forward()
        : block("msg_forward")
    {
        _in_port = message_port::make(
            "in", port_direction_t::INPUT);
        _in_port->register_callback([this](const std::string& msg) { this->handle_msg(msg); });
        add_port(_in_port);

        _out_port = message_port::make(
            "out", port_direction_t::OUTPUT);
        add_port(_out_port);

    }

private:
    message_port_sptr _in_port;
    message_port_sptr _out_port;

    void handle_msg(const std::string& msg)
    {
        gr_log_info(_logger, "got message: {}", msg);
        // _out_port->post(msg);
    }

};

} // namespace blocks
} // namespace gr
