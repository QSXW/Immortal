#pragma once

#include <cstdint>

#if defined( _MSC_VER )
#   define SL_ALIGNED(x) __declspec(align(x))
#	define SL_ALIGNED_STRUCT(x) struct __declspec(align(x))
#	define SL_ALIGNED_CLASS(x) class __declspec(align(x)))
#   define SL_ASSEMBLY __asm
#pragma warning(disable: 4996)

#elif defined( __GNUC__ )
#   define SL_ALIGNED(x) __attribute__((aligned(x)))
#	define SL_ALIGNED_STRUCT(x) struct __attribute__((aligned(x)))
#	define SL_ALIGNED_CLASS(x) class __attribute__((aligned(x)))
#   define SL_ASSEMBLY __asm__
#endif

namespace sl
{
#include <cassert>
#define SLASSERT(...) assert(__VA_ARGS__)


#define SLDEBUG defined( DEBUG) || defined( _DEBUG )

#if defined( WIN32 ) || defined( _WIN32 )
#   define WINDOWS
#   define SLSURFACE "VK_KHR_win32_surface"
#elif defined( __ANDROID__ )
#   define ANDROID
elif defined(__linux__)
#   define LINUX
#elif defined( __APPLE__ ) || defined( __MACH__ )
#   define APPLE
#endif

#ifndef NOEXCEPT
#define NOEXCEPT noexcept
#endif

#ifndef NODISCARD
#define NODISCARD [[nodiscard]]
#endif

#define BIT(x) (1 << (x))

#define U8(str) u8##str

#define SL_ARRAY_LEN(a) sizeof(a) / sizeof((a)[0])

using INT8   = char;
using UINT8  = unsigned char;
using INT16  = short;
using UINT16 = unsigned short;
using INT32  = int;
using UINT32 = unsigned int;
using INT64  = int64_t;
using UINT64 = uint64_t;

template <class T1, class T2>
inline constexpr bool typeof()
{
    return std::is_same_v<T1, T2>;
}

template <class T1, class T2, class T3>
inline constexpr bool typeof()
{
    if constexpr (std::is_same_v<T1, T2> || std::is_same_v<T1, T3>)
    {
        return true;
    }
    return false;
}

template <class T1, class T2, class T3, class T4>
inline constexpr bool typeof()
{
    if constexpr (std::is_same_v<T1, T2> ||
        std::is_same_v<T1, T3> ||
        std::is_same_v<T1, T4>)
    {
        return true;
    }
    return false;
}
}
