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
T Lerp(T start, T end, T t)
{
	return start + t * (end - start);
}

template <class T>
T CubicLerp(T a, T b, T t)
{
	return a + (3 * t * t - 2 * t * t * t) * (b - a);
}

template <class T>
T HermiteLerp(T a, T b, T t)
{
	T t2 = t * t;
	T t3 = t2 * t;
	return (2 * t3 - 3 * t2 + 1) * a + (t3 - 2 * t2 + t) * (b - a) + (-2 * t3 + 3 * t2) * b;
}

template <class T>
T Smoothstep(T a, T b, T t)
{
	t = t * t * (3 - 2 * t);
	return a + t * (b - a);
}

template <class T>
T CosineLerp(T a, T b, T t)
{
	T t2 = (1 - cos(t * PI)) / 2;
	return a * (1 - t2) + b * t2;
}

template <class T>
T ExponentialLerp(T a, T b, T t)
{
	if (t == 0.0f)
	{
		return a;
	}
	if (t == 1.0f)
	{
		return b;
    }
	return a * std::pow(b / a, t);
}

template <class T>
T QuadraticBezierLerp(T a, T b, T t)
{
	T u = 1 - t;
	return (u * u * a) + (2 * u * t * ((a + b) / 2)) + (t * t * b);
}

template <class T>
T InverseSmoothstep(T a, T b, T t)
{
	t = 0.5f - 0.5f * std::cos(PI * t);
	return a + (b - a) * t;
}

template <class T>
T PolynomialLerp(T a, T b, T t)
{
	return a * (1 - t) * (1 - t) * (1 - t) + b * t * t * t;
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
        numerator{ (int)n },
        denominator{ (int)d }
    {
        static_assert(std::is_arithmetic_v<T> && std::is_arithmetic_v<U>);
    }

    double Normalize() const
    {
        return (double)numerator / (double)denominator;
    }

    Rational operator *(const Rational &b)
    {
        Rational c;
		c.numerator   = numerator   * b.numerator;
		c.denominator = denominator * b.denominator;
		return c;
    }

    bool operator==(const Rational &other) const
    {
		return numerator == other.numerator && denominator == other.denominator;
    }

    int numerator;
    int denominator;
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
