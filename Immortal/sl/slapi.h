#pragma once

#include <cstdint>

#if defined( _MSC_VER )
#   define SL_ALIGNED(x) __declspec(align(x))
#	define SL_ALIGNED_STRUCT(x) struct __declspec(align(x))
#	define SL_ALIGNED_CLASS(x) class __declspec(align(x)))
#   define SL_ASSEMBLY __asm

#   define sl_disable_optimizations optimize("", off)
#   define sl_enable_optimizations  optimize("", on)

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

#define slptr *

#define SLASSERT(...) assert(__VA_ARGS__)

#define SLBIND(x) std::bind(&x, this, std::placeholders::_1)

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

template <class T>
inline constexpr void *CleanUpObject(T *ptr, int value = 0, size_t size = sizeof(T))
{
    return memset(ptr, value, size);
}

static uint32_t CyclicRedundancyCheck32(const uint8_t *message, uint32_t length)
{
    static constexpr uint32_t CRC32_GENERATOR_POLYNOMIAL = 0x04C11DB7;
    uint32_t CRC = (uint32_t)~0;

    for (int i = 0; i < length; i++)
    {
        CRC ^= (((uint32_t)message[i]) << 24);
        for (int j = 0; j < 8; j++)
        {
            uint32_t MSB = CRC >> 31;
            CRC <<= 1;
            CRC ^= (0 - MSB) & CRC32_GENERATOR_POLYNOMIAL;
        }
    }
    
    return CRC;
}

}
