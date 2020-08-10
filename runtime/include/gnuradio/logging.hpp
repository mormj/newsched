#pragma once

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"
#include <map>
#include <gnuradio/prefs.hpp>

/**
 * @brief GR Logging Macros and convenience functions
 *
 */
namespace gr {

typedef std::shared_ptr<spdlog::logger> logger_sptr;


class logging
{
    

public:
    static logger_sptr get_logger(const std::string& which)
    {
        static std::map<std::string, logger_sptr> logger_map;

        auto it = logger_map.find(which);
        if (it == logger_map.end()) {
            // TODO: Look in prefs to determine what type of logger for this name
            auto p = prefs::get_instance();

        }
        else
        {
            return it->second;
        }
    }
};


} // namespace gr