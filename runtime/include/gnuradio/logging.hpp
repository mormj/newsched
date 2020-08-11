#pragma once

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include <gnuradio/prefs.hpp>
#include <map>

/**
 * @brief GR Logging Macros and convenience functions
 *
 */
namespace gr {

typedef std::shared_ptr<spdlog::logger> logger_sptr;

typedef spdlog::level::level_enum logging_level_t;

enum class logger_type { console, basic, rotating, daily };
static std::unordered_map<std::string, logger_type> const logger_type_table = {
    { "console", logger_type::console },
    { "basic", logger_type::basic },
    { "rotating", logger_type::rotating },
    { "daily", logger_type::daily }
};

enum class logger_console_type { stdout, stderr };
static std::unordered_map<std::string, logger_console_type> const
    logger_console_type_table = { { "stdout", logger_console_type::stdout },
                                  { "stderr", logger_console_type::stderr } };

// forward declare the derived classes
struct logger_basic_config;
struct logger_console_config;

struct logger_config {
    YAML::Node _config;

    std::string id;
    logger_type type;

    logger_config(YAML::Node config)
    {
        _config = config;
        id = config["id"].as<std::string>();
        auto str = config["type"].as<std::string>();
        auto it = logger_type_table.find(str);
        if (it != logger_type_table.end()) {
            type = it->second;
        }
    }

    // unnecessary
    static std::shared_ptr<logger_config> parse(YAML::Node config)
    {
        // _config = config;
        // logger_config tmp(config);
        // if (tmp.type == logger_type::basic)
        //     return logger_basic_config::make(config);
        // else if (tmp.type == logger_type::console)
        //     return logger_console_config::make(config);
        // else
        //     return nullptr;
        return std::make_shared<logger_config>(logger_config(config));
    }
};

struct logger_console_config : logger_config {
    logger_console_type console_type;

    logger_console_config(YAML::Node config) : logger_config(config)
    {
        // type = logger_console_type_table[config["console_type"].as<std::string>()];
        auto str = config["console_type"].as<std::string>();
        auto it = logger_console_type_table.find(str);
        if (it != logger_console_type_table.end()) {
            console_type = it->second;
        }
    }

    static logger_sptr make(std::shared_ptr<logger_config> logger_config)
    {
        auto cfg = logger_console_config(logger_config->_config);

        if (cfg.console_type == logger_console_type::stdout) {
            return spdlog::stdout_color_mt(cfg.id);
        } else {
            return spdlog::stderr_color_mt(cfg.id);
        }
        // return std::make_shared<logger_console_config>(
        //     logger_console_config(logger_config._config));
    }
};

struct logger_basic_config : logger_config {
    std::string filename;

    logger_basic_config(YAML::Node config) : logger_config(config)
    {
        filename = config["filename"].as<std::string>();
    }

    static logger_sptr make(std::shared_ptr<logger_config> logger_config)
    {
        auto cfg = logger_basic_config(logger_config->_config);

        return spdlog::basic_logger_mt(cfg.id, cfg.filename);

        // return std::make_shared<logger_basic_config>(
        //     logger_basic_config(logger_config._config));
    }
};

class logging_config
// follows the structure of the yaml
{
public:
    logging_config() { parse_from_prefs(); }
    std::vector<std::shared_ptr<logger_config>> loggers;

private:
    void parse_from_prefs()
    {
        auto node = prefs::get_section("logging");
        for (auto info : node) {
            loggers.push_back(logger_config::parse(info));
        }
    }
};

class logging
{

public:
    static logger_sptr get_logger(const std::string& which)
    {
        static std::map<std::string, logger_sptr> logger_map; // singleton map of loggers
        logger_sptr requested_logger = nullptr;

        auto it = logger_map.find(which);
        if (it == logger_map.end()) {
            // TODO: Look in prefs to determine what type of logger for this name
            // auto p = prefs::get_instance();

            // Find the configuration for this named logger
            logging_config cfg;
            auto it = std::find_if(
                cfg.loggers.begin(),
                cfg.loggers.end(),
                [&](std::shared_ptr<logger_config> lg) { return lg->id == which; });

            // Found the configuration, now create the logger
            if (it != cfg.loggers.end()) {
                switch ((*it)->type) {
                case logger_type::basic:
                    requested_logger = logger_basic_config::make(*it);
                    break;
                case logger_type::console:
                    requested_logger = logger_console_config::make(*it);
                    break;
                }

            } else {
                std::cout << "Logger: " << which << " not found in configuration"
                          << std::endl;
            }

        } else {
            requested_logger = it->second;
        }

        logger_map[which] = requested_logger;
        return requested_logger;
    }

};

inline void set_level(logger_sptr logger, logging_level_t log_level)
{
    logger->set_level(log_level);
}
// inline logging_level_t get_level(logger_sptr logger)
// {
//     return logger->get_level();
// }


inline void gr_log_debug(logger_sptr logger, const std::string& msg)
{
    logger->debug(msg); 
}
inline void gr_log_info(logger_sptr logger, const std::string& msg)
{
    logger->info(msg); 
}
inline void gr_log_trace(logger_sptr logger, const std::string& msg)
{
    logger->trace(msg); 
}
inline void gr_log_warn(logger_sptr logger, const std::string& msg)
{
    logger->warn(msg); 
}
inline void gr_log_error(logger_sptr logger, const std::string& msg)
{
    logger->error(msg); 
}
inline void gr_log_critical(logger_sptr logger, const std::string& msg)
{
    logger->critical(msg); 
}


// Do we need or want these macros if we have inline functions
#define GR_LOG_SET_LEVEL(logger, level) logger->set_level(level);
// #define GR_LOG_GET_LEVEL(logger, level) gr::logger_get_level(logger, level)
#define GR_LOG_DEBUG(logger, msg) \
    {                             \
        logger->debug(msg);       \
    }

#define GR_LOG_INFO(logger, msg) \
    {                            \
        logger->info(msg);       \
    }

#define GR_LOG_TRACE(logger, msg) \
    {                             \
        logger->trace(msg);       \
    }

#define GR_LOG_WARN(logger, msg) \
    {                            \
        logger->warn(msg);       \
    }

#define GR_LOG_ERROR(logger, msg) \
    {                             \
        logger->error(msg);       \
    }

#define GR_LOG_CRIT(logger, msg) \
    {                            \
        logger->critical(msg);   \
    }


#define GR_LOG_ERRORIF(logger, cond, msg) \
    {                                     \
        if ((cond)) {                     \
            logger->error(msg);           \
        }                                 \
    }

#define GR_LOG_ASSERT(logger, cond, msg) \
    {                                    \
        if (!(cond)) {                   \
            logger->error(msg);          \
            assert(0);                   \
        }                                \
    }

} // namespace gr