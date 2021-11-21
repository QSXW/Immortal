#pragma once

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
inline constexpr T dcast(O o)
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
