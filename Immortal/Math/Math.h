#pragma once

#include <cmath>

namespace Immortal
{
namespace Math
{

constexpr const double PI = 3.14159265358979323846;

#define FW(N, F) \
static inline double N(double x) noexcept \
{ \
    return std::F(x); \
} \
static inline float N(float x) noexcept \
{ \
    return std::F(x); \
}

FW(Cos, cos)
FW(Sin, sin)
FW(Tan, tan)
FW(Absolute, fabsf)

template <class T>
T Lerp(T a, T b, T w)
{
    return a + (b - a) * w;
}

struct Rational
{
    Rational() :
        numerator{ 0 },
        denominator{ 1 }
    {

    }
    
    template <class T>
    Rational(T n, T d) :
        numerator{ (int64_t)n },
        denominator{ (int64_t)d }
    {
        static_assert(std::is_arithmetic_v<T>);
    }

    int64_t numerator;
    int64_t denominator;
};

}

using Rational = Math::Rational;

}
