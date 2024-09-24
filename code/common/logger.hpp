#pragma once
#include <iostream>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async.h>

namespace logging
{
    class logger
    {
    public:
        static void init_logger(bool mode, const std::string &file, int32_t level)
        {
            if (mode == false)
            {
                g_default_logger = spdlog::stdout_color_mt("default-logger");
                g_default_logger->set_level(spdlog::level::level_enum::trace);
                g_default_logger->flush_on(spdlog::level::level_enum::trace);
            }
            else
            {
                g_default_logger = spdlog::basic_logger_mt("file-logger", file);
                g_default_logger->set_level((spdlog::level::level_enum)level);
                g_default_logger->flush_on((spdlog::level::level_enum)level);
            }
            g_default_logger->set_pattern("[%n][%H:%M:%S][%t][%-8l]%v");
        }
        static std::shared_ptr<spdlog::logger> g_default_logger;
    };
    std::shared_ptr<spdlog::logger> logger::g_default_logger;
} // namespace log

#define LOG_TRACE(format, ...) logging::logger::g_default_logger->trace(std::string("[{}:{}] ") + format, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...) logging::logger::g_default_logger->debug(std::string("[{}:{}] ") + format, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_INFO(format, ...) logging::logger::g_default_logger->info(std::string("[{}:{}] ") + format, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_WARN(format, ...) logging::logger::g_default_logger->warn(std::string("[{}:{}] ") + format, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) logging::logger::g_default_logger->error(std::string("[{}:{}] ") + format, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_CRIT(format, ...) logging::logger::g_default_logger->critical(std::string("[{}:{}] ") + format, __FILE__, __LINE__, ##__VA_ARGS__)