#pragma once

namespace sl
{
inline namespace cast
{
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
        static_assert(std::is_base_of<T, O>());
        return dynamic_cast<T>(o);
    }

    template <class T, class O>
    inline constexpr T ccast(O o)
    {
        return const_cast<T>(o);
    }   
}
}
