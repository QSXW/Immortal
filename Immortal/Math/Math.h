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
FW(Absolute, fabs)

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

    template <class T, class U>
    Rational(T n, U d) :
        numerator{ (int64_t)n },
        denominator{ (int64_t)d }
    {
        static_assert(std::is_arithmetic_v<T> && std::is_arithmetic_v<U>);
    }

    double Normalize() const
    {
        return (double)numerator / (double)denominator;
    }

    int64_t numerator;
    int64_t denominator;
};

/** Gaussian Function
 * 
 *  @ret The probability
 */
static inline float Gaussian(float sigma, float x)
{
    return 1.0 / (sqrt(2.0 * PI) * sigma) * exp(-(x * x) / (2.0 * sigma * sigma));
}

/** Integrate based on Simpson Rule
 */
template <class T>
static inline float Integrate(float sigma, float a, float b, T &&func)
{
    return (b - a) * (1.0 / 6.0) * ((func(a) + 4.0 * func((a + b) * 0.5) + func(b)));
}

/** Generate Gaussian Kernel
 *  
 *  @kernal the size should be half the required convolution kernal size
 *  @sigma needed for gaussian function
 * 
 */
static inline void GenerateGaussianKernal(float *kernal, size_t kernalSize, float sigma)
{
    auto gaussian = [=](float x) { return Gaussian(sigma, x);  };

    float sum = 0;
	for (int i = 0; i < kernalSize; i++)
    {
        kernal[i] = Integrate(sigma, i - 0.5f, i + 0.5f, gaussian);
        if (i)
        {
            sum += kernal[i] * 2.0f;
        }
        else
        {
            sum += kernal[i];
        }
    }

    /** Normalize
     *  For the integral upper bound and lower bound are limited form (-INF, +INF)
     *  to (-KernalSize, +KernalSize), so cumulative probability is not able to
     *  reach 1.0f.
     */
    sum = 1.0f / sum;
	for (int i = 0; i < kernalSize; i++)
    {
		kernal[i] *= sum;
    }
}

}

using Rational = Math::Rational;

}
