#include "impch.h"
#include "Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace Immortal {

	Ref<spdlog::logger> Log::logger;
	Ref<spdlog::logger> Log::sClientLogger;

	void Log::Init()
	{
		std::vector<spdlog::sink_ptr> logSinks;
		logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Immortal.log", true));

		logSinks[0]->set_pattern("%n: [%^%l%$][%T]: %v");
		logSinks[1]->set_pattern("[%T] [%l] %n: %v");

		logger = std::make_shared<spdlog::logger>("Immortal", begin(logSinks), end(logSinks));
		spdlog::register_logger(logger);
		logger->set_level(spdlog::level::trace);
		logger->flush_on(spdlog::level::trace);

		sClientLogger = std::make_shared<spdlog::logger>("APP", begin(logSinks), end(logSinks));
		spdlog::register_logger(sClientLogger);
		sClientLogger->set_level(spdlog::level::trace);
		sClientLogger->flush_on(spdlog::level::trace);
	}

}