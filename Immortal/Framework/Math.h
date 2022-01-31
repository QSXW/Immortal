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

}
}
