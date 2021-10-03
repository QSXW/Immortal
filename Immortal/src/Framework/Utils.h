#pragma once

#include <locale>
#include <codecvt>

namespace Immortal
{
namespace Utils
{
template <class T>
inline constexpr bool IsOdd(const T &x)
{
    static_assert(std::is_integral<T, int>());
    return 0x1 & x;
}

template <class T>
inline constexpr bool IsEven(const T &x)
{
    return !IsOdd(x);
}

template <class T>
inline constexpr const T &Max(const T &x, const T &y)
{
    return x > y ? x : y;
}

template <class T>
inline constexpr const T &Min(const T &x, const T &y)
{
    return x < y ? x : y;
}

template <class T, T min, T max>
inline constexpr void Clamp(T &v)
{
    v = Min(Max(v, min), max);
}

template <class T>
inline constexpr void Clamp(T &v, const T &min, const T &max)
{
    v = Min(Max(v, min), max);
}

template <int min, int max>
inline constexpr void Clamp(int &v) 
{
    Clamp<int, min, max>(v);
}

template <class T, T m>
inline constexpr T Mod(T &v)
{
    static_assert(Type::IsIntegral<T>());
    return v & (m - 1);
}

template <int m>
inline constexpr int Mod(int &v) 
{
    return Mod<int, m>(v);
}

template <class T, T v>
inline constexpr T Power2() 
{
    return ((size_t)0x1) << v;
}

template <class T>
inline constexpr T Power2(T &v) 
{
    return ((size_t)0x1) << v;
}

static inline std::wstring s2ws(const std::string &str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    return converter.from_bytes(str);
}

static inline std::string ws2s(const std::wstring &wstr)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    return converter.to_bytes(wstr);
}

}
}
