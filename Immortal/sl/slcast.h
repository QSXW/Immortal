#pragma once

#include "sl.h"

namespace sl
{

template <class T, class O>
inline constexpr T CAST(O o)
{
    return (T)o;
}

template <class T, class O>
inline constexpr T rcast(O o)
{
    return reinterpret_cast<T>(o);
}

template <class T, class O>
inline constexpr T ncast(O o)
{
    return static_cast<T>(o);
}

template <class T, class O>
inline constexpr T bcast(O o)
{
    return *(reinterpret_cast<T*>(&o));
}

template <class T, class O>
inline constexpr T dcast(O &&o)
{
    return dynamic_cast<T>(o);
}

template <class T, class O>
inline constexpr T ccast(O o)
{
    return const_cast<T>(o);
}

template <class T, class O>
inline constexpr T ecast(O o)
{
    return reinterpret_cast<T>((void *)o);
}

template <class T, class O>
inline constexpr T U32(O o)
{
    return static_cast<uint32_t>(o);
}

template <class T, class O>
inline constexpr T U64(O o)
{
    return static_cast<uint64_t>(o);
}

template <class T>
inline constexpr Anonymous Anonymize(T &object)
{
    if constexpr (std::is_pointer<T>())
    {
        return (Anonymous)(object);
    }
    else
    {
        return (Anonymous)(&object);
    }
}

template <class T, class P = T*, class R = T&>
inline constexpr auto Deanonymize(Anonymous anonymous)
{
    if constexpr (std::is_pointer<T>() || std::is_reference<T>())
    {
        return reinterpret_cast<T>(anonymous);
    }
    return reinterpret_cast<R>(*reinterpret_cast<P>(anonymous));
}

template <class T>
inline constexpr uint32_t U32(T num)
{
    return static_cast<uint32_t>(num);
}

template <class T>
inline constexpr uint64_t U64(T num)
{
    return static_cast<uint64_t>(num);
}

template <class T>
inline constexpr T RemoveConst(const T o)
{
    return const_cast<T>(o);
}

inline bool Equals(const void *lhs, const void *rhs)
{
    return CAST<size_t>(lhs) == CAST<size_t>(rhs);
}

}
