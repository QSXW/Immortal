#pragma once

#include <cstdint>

namespace Immortal
{
namespace Vision
{

namespace ColorSpace
{

template <class T>
struct Vector
{
    using Ref = T*;
    union { Ref x, r; };
    union { Ref y, g; };
    union { Ref z, b; };
    union { Ref w, a;    };

    Ref &operator[](size_t index)
    {
        switch (index)
        {
        case  0: return x;
        case  1: return y;
        case  2: return z;
        case  3: return w;
        default: SLASSERT(0); return x;
        }
    }
};

enum class CoefficientType : size_t
{
    REC601 = 0,
    REC709
};

static float Coefficients[][4] = {
    {  0.299,  0.587, 0.114, 0 }, /* ITU-R Rec. 601 */
    { 0.2125, 0.7157, 0.721, 0 }, /* ITU-R Rec. 709 (1990) */
};

template <CoefficientType type>
float* Get()
{
    return Coefficients[static_cast<size_t>(type)];
}

template <class T, ColorSpace::CoefficientType C>
inline constexpr void rgb2yuv(T &y, T &u, T &v, const T &r, const T &g, const T &b)
{
    auto &[lr, lg, lb, la] = ColorSpace::Coefficients[static_cast<size_t>(C)];
    y = lr * r + lg * g + lb * b;
    u = (((T)(~0)) >> 1) + (b - y) / (2 - 2 * lb);
    v = (((T)(~0)) >> 1) + (r - y) / (2 - 2 * lr);
}

template <class T>
inline constexpr void yuv2rgb(T &r, T &g, T &b, const T &y, const T &u, const T &v)
{
    double cb = u - 128;
    double cr = v - 128;
    r = y + 1.402   * cr;
    g = y - 0.34414 * cb - 0.71414 * cr;
    b = y + 1.772   * cb;
}

void RGBA8ToYUVA4444(uint8_t *dst, const uint8_t *src, size_t size);

void RGBA8ToYUVA4444_AVX2(uint8_t *dst, const uint8_t *src, size_t size);

template <class T, class U>
void YUV444PToRGBA8(Vector<T> &dst, Vector<U> &src, size_t width, size_t height, size_t padding = 0)
{
    auto y = src.x, u = src.y, v = src.z;
    auto dstptr = dst.x;
    for (size_t i = 0; i < height; i++)
    {
        for (size_t j = 0; j < width; j++)
        {
            yuv2rgb(dstptr[0], dstptr[1], dstptr[2], *y++, *u++, *v++);
            dstptr[3] = 0xff;
            dstptr += 4;
        }
        y += padding;
        u += padding;
        v += padding;
    }
}

template <class T, class U>
void YUV420PToRGBA8(Vector<T> &dst, Vector<U> &src, size_t width, size_t height, size_t padding = 0)
{
    auto y = src.x, u = src.y, v = src.z;
    auto dstptr = dst.x;

    auto linesize = width * 4;
    auto rows = width / 2;
    auto cols = height / 2;
    for (size_t i = 0; i < cols; i++, dstptr += linesize)
    {
        for (size_t j = 0; j < rows; j++, y += 2, u++, v++, dstptr += 8)
        {
            auto r1 = dstptr;
            auto r2 = dstptr + linesize;
            yuv2rgb(r1[0], r1[1], r1[2], y[        0], *u, *v); r1[3] = 0xff;
            yuv2rgb(r1[4], r1[5], r1[6], y[        1], *u, *v); r1[7] = 0xff;
            yuv2rgb(r2[0], r2[1], r2[2], y[width + 0], *u, *v); r2[3] = 0xff;
            yuv2rgb(r2[4], r2[5], r2[6], y[width + 1], *u, *v); r2[7] = 0xff;
        }
        y += width + padding * 2;
        u += padding;
        v += padding;
    }
}

};

}
}
