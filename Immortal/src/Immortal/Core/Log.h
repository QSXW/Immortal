#pragma once

#include <memory>
#include <cstdio>

#include "ImmortalCore.h"

#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

namespace Immortal {

	class IMMORTAL_API Log
	{
	public:
		static void Init();

		static Ref<spdlog::logger>& GetCoreLogger()
		{ 
			return sCoreLogger; 
		}
		static Ref<spdlog::logger>& GetClientLogger()
		{ 
			return sClientLogger;
		}

	private:
		static Ref<spdlog::logger> sCoreLogger;
		static Ref<spdlog::logger> sClientLogger;
	};

}

#if defined( DEBUG ) | defined( _DEBUG )
	#define __FILENAME__ (static_cast<const char *>(__FILE__) + 30)
	#define IM_CORE_WARN(...)		::Immortal::Log::GetCoreLogger()->warn(__VA_ARGS__);
	#define IM_CORE_INFO(...)		::Immortal::Log::GetCoreLogger()->info(__VA_ARGS__);
	#define IM_CORE_ERROR(...)		::Immortal::Log::GetCoreLogger()->error("[{}:{}] {}", __FILENAME__, __LINE__, fmt::format(__VA_ARGS__));
	#define IM_CORE_DEBUG(...)		::Immortal::Log::GetCoreLogger()->debug(__VA_ARGS__);
	#define IM_CORE_TRACE(...)		::Immortal::Log::GetCoreLogger()->trace(__VA_ARGS__);
	#define IM_CORE_CRITICAL(...)   ::Immortal::Log::GetCoreLogger()->critical(__VA_ARGS__);
	#define IM_APP_WARN(...)		::Immortal::Log::GetClientLogger()->warn(__VA_ARGS__);
	#define IM_APP_INFO(...)		::Immortal::Log::GetClientLogger()->info(__VA_ARGS__);
	#define IM_APP_ERROR(...)		::Immortal::Log::GetClientLogger()->error(__VA_ARGS__);
	#define IM_APP_TRACE(...)		::Immortal::Log::GetClientLogger()->trace(__VA_ARGS__);
	#define IM_APP_CRITICAL(...)	::Immortal::Log::GetClientLogger()->critical(__VA_ARGS__);
#else
	#define IM_CORE_WARN(...)	
	#define IM_CORE_INFO(...)	
	#define IM_CORE_ERROR(...)	
	#define IM_CORE_DEBUG(...)
	#define IM_CORE_TRACE(...)	
	#define IM_CORE_FATAL(...)	
	#define IM_CORE_CRITICAL(...)
	#define IM_APP_WARN(...)		
	#define IM_APP_INFO(...)		
	#define IM_APP_ERROR(...)		
	#define IM_APP_TRACE(...)		
	#define IM_APP_FATAL(...)
	#define IM_APP_CRITICAL(...)
#endif