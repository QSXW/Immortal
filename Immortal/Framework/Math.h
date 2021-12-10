#pragma once

#include <cmath>

#define IMMORTAL_CSTD ::

namespace Immortal
{
namespace Math
{

constexpr const double PI = 3.14159265358979323846;

static inline double Cos(double x) noexcept
{
    return IMMORTAL_CSTD cos(x);
}

static inline float Cos(float x) noexcept
{
    return IMMORTAL_CSTD cos(x);
}

static inline double Sin(double x) noexcept
{
    return IMMORTAL_CSTD sin(x);
}

static inline float Sin(float x) noexcept
{
    return IMMORTAL_CSTD sin(x);
}

static inline float Absolute(float x) noexcept
{
    return IMMORTAL_CSTD fabsf(x);
}

}
}
