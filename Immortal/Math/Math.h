#pragma once

#include <cmath>
#include <numeric>

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
        numerator{ (int64_t)n },
        denominator{ (int64_t)d }
    {
        static_assert(std::is_arithmetic_v<T> && std::is_arithmetic_v<U>);
    }

    double Normalize() const
    {
        return (double)numerator / (double)denominator;
    }

    Rational operator *(const Rational &b) const
    {
        Rational c;
		c.numerator   = numerator   * b.numerator;
		c.denominator = denominator * b.denominator;
		return c;
    }

    Rational operator*(const int64_t v) const
	{
		Rational c;
		c.numerator   = numerator * v;
		c.denominator = denominator;
		return c;
	}

    Rational operator/(const Rational &b) const
	{
		Rational c;
		c.numerator   = b.denominator;
		c.denominator = b.numerator;
		return *this * c;
	}

    Rational operator+(const Rational &other) const
	{
        if (denominator == other.denominator)
        {
			return {numerator + other.numerator, denominator};
        }
        else
        {
			return Rational(numerator * other.denominator + other.numerator * denominator, denominator * other.denominator);
        }
	}

    Rational operator-(const Rational &other) const
	{
		if (denominator == other.denominator)
		{
			return {numerator - other.numerator, denominator};
		}
		else
		{
			return Rational(numerator * other.denominator - other.numerator * denominator, denominator * other.denominator);
		}
	}

    Rational &Reduce()
    {
        if (!denominator)
        {
			return *this;
        }

        int64_t v = std::gcd(std::abs(numerator), std::abs(denominator));
		numerator   /= v;
		denominator /= v;

        if (denominator < 0)
        {
			numerator   = -numerator;
			denominator = -denominator;
        }

		return *this;
    }

    bool operator==(const Rational &other) const
    {
		return numerator == other.numerator && denominator == other.denominator;
    }

    bool operator<=(const Rational &other) const
	{
		int64_t a = numerator * other.denominator;
		int64_t b = other.numerator * denominator;
		return a <= b;
	}

    bool operator>=(const Rational &other) const
	{
		int64_t a = numerator * other.denominator;
		int64_t b = other.numerator * denominator;
		return a >= b;
	}

    bool operator<(const Rational &other) const
    {
		int64_t a  = numerator * other.denominator;
		int64_t b = other.numerator * denominator;
		return a < b;
    }

    bool operator>(const Rational &other) const
	{
		int64_t a = numerator * other.denominator;
		int64_t b = other.numerator * denominator;
		return a > b;
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
