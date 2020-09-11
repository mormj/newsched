#pragma once

#include <gnuradio/scheduler_message.hpp>
#include <gnuradio/node.hpp>
#include <pmt/pmt.hpp>
namespace gr {

enum class message_port_direction_t { INPUT, OUTPUT };
struct async_message : public scheduler_message {
    nodeid_t blkid;
    pmt::pmt_sptr data;
    std::string port_name;

    async_message(nodeid_t blkid_,
                  const std::string& port_name_,
                  pmt::pmt_sptr data_)
        : scheduler_message(scheduler_message_t::ASYNC_MESSAGE),
          blkid(blkid_),
          port_name(port_name_),
          data(data_)
    {
    }
};

typedef std::function<void(pmt::pmt_t)> block_async_msg_fcn;


} // namespace gr