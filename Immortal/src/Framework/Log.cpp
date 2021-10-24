#include "impch.h"
#include "Log.h"

#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>

namespace Immortal
{

std::shared_ptr<spdlog::logger> LOG::logger;

void LOG::INIT()
{
    spdlog::init_thread_pool(8192, 1);
    std::vector<spdlog::sink_ptr> logSinks;
    logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/Immortal.log"));

    logSinks[0]->set_pattern("%n: [%^%l%$][%T]: %v");
    logSinks[1]->set_pattern("[%T] [%l] %n: %v");

    logger = std::make_shared<spdlog::async_logger>("Immortal", logSinks.begin(), logSinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::trace);
}
}
