#pragma once

#include "ImmortalCore.h"
#include "Log.h"
#include <filesystem>

#ifdef HZ_ENABLE_ASSERTS
	// Alteratively we could use the same "default" message for both "WITH_MSG" and "NO_MSG" and
	// provide support for custom formatting by concatenating the formatting string instead of having the format inside the default message
	#define IM_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { IM##type##ERROR(msg, __VA_ARGS__); IM_DEBUGBREAK(); } }
	#define IM_INTERNAL_ASSERT_WITH_MSG(type, check, ...) IM_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
	#define IM_INTERNAL_ASSERT_NO_MSG(type, check) IM_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", IM_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)
			
	#define IM_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
	#define IM_INTERNAL_ASSERT_GET_MACRO(...) IM_EXPAND_MACRO( IM_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, IM_INTERNAL_ASSERT_WITH_MSG, IM_INTERNAL_ASSERT_NO_MSG) )

	// Currently accepts at least the condition and one additional parameter (the message) being optional
	#define IM_APP_ASSERT(...) IM_EXPAND_MACRO( HZ_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
	#define SLASSERT(x, ...) { if(!(x)) { IM_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#else
	#define IM_ASSERT(...)
	#define SLASSERT(...)
#endif