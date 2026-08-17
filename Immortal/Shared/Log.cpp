#include "Log.h"

#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>

namespace Immortal
{

std::shared_ptr<spdlog::logger> LOG::logger;

void LOG::Setup(bool async, const std::filesystem::path &path)
{
    std::vector<spdlog::sink_ptr> logSinks;

    logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    logSinks.back()->set_pattern("[%T][%^%l%$] %v");

    if (!path.empty())
	{
		std::string spath = path.string();
		logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(spath));
		logSinks.back()->set_pattern("[%T][%l] %n: %v");
	}

    if (async)
    {
        spdlog::init_thread_pool(8192, 1);
        logger = std::make_shared<spdlog::async_logger>("Immortal", logSinks.begin(), logSinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    }
    else
    {
        logger = std::make_shared<spdlog::logger>("Immortal", logSinks.begin(), logSinks.end());
    }

    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::trace);
}

void LOG::Release()
{
    logger.reset();
}

}
