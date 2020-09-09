#pragma once

#include <gnuradio/flowgraph_monitor.hpp>
#include <atomic>

class fgm_adapter_direct : public flowgraph_monitor
{
    

};

#pragma once


#include <gnuradio/buffer.hpp>
#include <gnuradio/domain_adapter.hpp>
namespace gr {


/**
 * @brief Uses direct access to the shared pointer to the flowgraph for access to the fgm queue
 */
class fgm_adapter_direct_svr : public flowgraph_monitor
{
private:
    std::thread d_thread;

public:
    typedef std::shared_ptr<fgm_adapter_direct_svr> sptr;
    static sptr make()
    {
        auto ptr =
            std::make_shared<fgm_adapter_direct_svr>(fgm_adapter_direct_svr());

        ptr->start_thread(ptr); // start thread with reference to shared pointer

        return ptr;
    }

    fgm_adapter_direct_svr()
        : flowgraph_monitor()
    {
    }

    void start_thread(sptr ptr) { d_thread = std::thread(run_thread, ptr); }

    static void run_thread(sptr top)
    {
        while (true) {
            {
                
            }
        }
    }
};


class fgm_adapter_direct_cli : public flowgraph_monitor
{
private:
    flowgraph_monitor_sptr remote = nullptr;

public:
    typedef std::shared_ptr<fgm_adapter_direct_cli> sptr;
    static sptr make(direct_sync_sptr sync, port_sptr other_port)
    {
        auto ptr =
            std::make_shared<fgm_adapter_direct_cli>(fgm_adapter_direct_cli(sync));

        return ptr;
    }
    fgm_adapter_direct_cli(direct_sync_sptr sync)
        : domain_adapter(buffer_location_t::LOCAL), p_sync(sync)
    {
    }


    void set_remote()
    {

    }

};

};


} // namespace gr