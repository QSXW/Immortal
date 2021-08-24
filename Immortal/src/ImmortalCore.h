#pragma once

#ifndef __IMMORTAL_CORE_H__
#define __IMMORTAL_CORE_H__

#define UNICODE

#include "impch.h"
#include <cstdint>
#include <memory>

#if WIN32
#    define IMMORTAL_PLATFORM_SURFACE "VK_KHR_win32_surface"
#endif

#pragma warning( disable: 4251 )
#pragma warning( disable: 4996 )
#pragma warning( disable: 4006 )
#pragma warning( disable: 26812 )

#define IMMORTAL_PLATFORM_WINDOWS

#if defined IMMORTAL_PLATFORM_WINDOWS
	#ifdef IMMORTAL_BUILD_NO_STATIC
		#if defined IMMORTAL_BUILD_DLL
			#define IMMORTAL_API __declspec(dllexport)
		#else
			#define IMMORTAL_API __declspec(dllimport
		#endif
	#else
		#define IMMORTAL_API
	#endif
#else
	#error Only support Windows!
#endif

#ifndef NOEXCEPT
#define NOEXCEPT noexcept
#endif

#ifndef NODISCARD
#define NODISCARD [[nodiscard]]
#endif

#define BIT(x) (1 << (x))
#define UNICODE8(str) u8##str
#define ARRAY_LEN(a) sizeof(a) / sizeof((a)[0])
#define IM_BIND_EVENT_FUNC(x) std::bind(&x, this, std::placeholders::_1)

namespace Immortal {

	using INT8   = int8_t;
	using UINT8  = uint8_t;
	using INT16  = int16_t;
	using UINT16 = uint16_t;
	using INT32  = int32_t;
	using UINT32 = uint32_t;
	using INT64  = int64_t;
	using UINT64 = uint64_t;

	template <class T>
	using Scope = std::unique_ptr<T>;

	template <class T>
	using Unique = Scope<T>;

	template <class T, class ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}


	template <class T, class ... Args>
	constexpr Unique<T> MakeUnique(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template <class T>
	using Ref = std::shared_ptr<T>;
	
	template <class T, class ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	namespace Utils
	{
		extern "C" static void *SafeChunk;

		/**
	     * @brief Use to Avoid return warning. This Value should never be used.
	     *
	     */
		template <class T>
		constexpr inline T NullValue()
		{
			return *(T*)SafeChunk;
		}
	}
}

#include "Framework/Vector.h"

#include "Framework/Log.h"

#include <cassert>
#define SLASSERT(...) assert(__VA_ARGS__)

#define IMMORTAL_CHECK_DEBUG defined( DEBUG) || defined( _DEBUG )
#define IMMORTAL_END_CHECK endif

#endif /* __IMMORTAL_CORE_H__ */
