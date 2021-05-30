#pragma once

namespace Immortal {

    namespace Type
    {
        template <class T, T v>
        struct IntegralConstant
        {
            static constexpr T value = v;
            using ValueType = T;
            using type = IntegralConstant<T, v>;
            constexpr operator ValueType() const noexcept { return value; }
            constexpr ValueType operator()() const noexcept { return value; }
        };

        using TrueType = IntegralConstant<bool, true>;
        using FalseType = IntegralConstant<bool, false>;

        template <class, class>
        struct IsSame : public FalseType { };

        template <class T>
        struct IsSame<T, T> : public TrueType { };

        template <class T>
        inline constexpr bool IsIntegral() noexcept
        {
            return IsSame<T, char>()
                | IsSame<T, unsigned char>()
                | IsSame<T, short>()
                | IsSame<T, unsigned short>()
                | IsSame<T, int>()
                | IsSame<T, unsigned int>()
                | IsSame<T, long long>()
                | IsSame<T, unsigned long long>();
        }
    }

    namespace Utils
    {
        template <class T>
        constexpr inline bool IsOdd(const T &x) noexcept
        {
            static_assert(Type::IsIntegral<T>());
            return 0x1 & x;
        }

        template <class T>
        constexpr inline bool IsEven(const T &x) noexcept
        {
            return !IsOdd(x);
        }

        template <class T>
        constexpr inline const T &Max(const T &x, const T &y) noexcept
        {
            return x > y ? x : y;
        }

        template <class T>
        constexpr inline const T &Min(const T &x, const T &y) noexcept
        {
            return x < y ? x : y;
        }

        template <class T, T min, T max>
        constexpr inline void Clamp(T &v) noexcept
        {
            v = Min(Max(v, min), max);
        }

        template <class T>
        constexpr inline void Clamp(T &v, const T &min, const T &max) noexcept
        {
            v = Min(Max(v, min), max);
        }

        template <int min, int max>
        constexpr inline void Clamp(int &v) noexcept
        {
            Clamp<int, min, max>(v);
        }

        template <class T, T m>
        constexpr inline T Mod(T &v) noexcept
        {
            static_assert(Type::IsIntegral<T>());
            return v & (m - 1);
        }

        template <int m>
        constexpr inline int Mod(int &v) noexcept
        {
            return Mod<int, m>(v);
        }

        template <class T, T v>
        constexpr inline T Power2() noexcept
        {
            return ((size_t)0x1) << v;
        }

        template <class T>
        constexpr inline T Power2(T &v) noexcept
        {
            return ((size_t)0x1) << v;
        }
    }


}