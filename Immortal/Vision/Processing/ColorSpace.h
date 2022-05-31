/**
 * Copyright (C) 2021-2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include <cstdint>
#include "Core.h"
#include "Framework/Utils.h"

namespace Immortal
{
namespace Vision
{

/** Vector in Color Space: 
 * 
 * Use to identifier a vector in color space
 * [x, y, z, w] <-> [r, g, b, a] <-> [c, m, y, k] <-> [h, l, s, #]
 */
template <class T>
struct CVector
{
    CVector() :
        x{},
        y{},
        z{},
        w{},
        linesize{}
    {}

    using Ref = T*;
    union { Ref x, r; };
    union { Ref y, g; };
    union { Ref z, b; };
    union { Ref w, a;    };

    int linesize[3];

    Ref &operator[](size_t index)
    {
        switch (index)
        {
        case  0: return x;
        case  1: return y;
        case  2: return z;
        case  3: return w;
        default: THROWIF(true, SError::OutOfBound); return x;
        }
    }
};

#define PAD(x, a) (x) & ((a) - 1)

enum class CoefficientType : size_t
{
    REC601 = 0,
    REC709
};

static float Coefficients[][4] = {
    {  0.299,  0.587, 0.114, 0 }, /* ITU-R Rec. 601 */
    { 0.2125, 0.7157, 0.721, 0 }, /* ITU-R Rec. 709 (1990) */
};

template <class T, CoefficientType C>
inline constexpr void rgb2yuv(T &y, T &u, T &v, const T &r, const T &g, const T &b)
{
    auto &[lr, lg, lb, la] = Coefficients[static_cast<size_t>(C)];
    y = lr * r + lg * g + lb * b;
    u = (((T)(~0)) >> 1) + (b - y) / (2 - 2 * lb);
    v = (((T)(~0)) >> 1) + (r - y) / (2 - 2 * lr);
}

template <class T>
inline constexpr void yuv2rgb(T &r, T &g, T &b, const T &y, const T &u, const T &v)
{
#define CLIP(x) std::min(std::max(x, 0.0), 255.0)

    double cb = u - 128;
    double cr = v - 128;
    r = CLIP(y + 1.402 * cr);
    g = CLIP(y - 0.34414 * cb - 0.71414 * cr);
    b = CLIP(y + 1.772 * cb);
}

void RGBA8ToYUVA4444(uint8_t *dst, const uint8_t *src, size_t size);

void RGBA8ToYUVA4444_AVX2(uint8_t *dst, const uint8_t *src, size_t size);

template <class T, class U>
void YUV444PToRGBA8(CVector<T> &dst, CVector<U> &src, size_t width, size_t height, CoefficientType type = CoefficientType::REC601)
{
    auto y = src.x, u = src.y, v = src.z;
    auto dstptr   = dst.x;

    for (size_t i = 0; i < height; i++)
    {
        for (size_t j = 0; j < width; j++, dstptr += 4)
        {
            yuv2rgb(dstptr[0], dstptr[1], dstptr[2], y[j], u[j], v[j]);
            dstptr[3] = 0xff;
        }
        y += src.linesize[0];
        u += src.linesize[0];
        v += src.linesize[0];
    }
}

template <class T, class U>
void YUV420PToRGBA8(CVector<T> &dst, CVector<U> &src, size_t width, size_t height, CoefficientType type = CoefficientType::REC601)
{
    auto y = src.x, u = src.y, v = src.z;
    auto dstptr = dst.x;

    auto linesize = width * 4;
    auto rows = width / 2;
    auto cols = height / 2;
    
    for (size_t i = 0; i < cols; i++, dstptr += linesize)
    {
        for (size_t j = 0; j < rows; j++, dstptr += 8)
        {
            auto r1 = dstptr;
            auto r2 = dstptr + linesize;
            yuv2rgb(r1[0], r1[1], r1[2], y[2 * j                   + 0], u[j], v[j]); r1[3] = 0xff;
            yuv2rgb(r1[4], r1[5], r1[6], y[2 * j                   + 1], u[j], v[j]); r1[7] = 0xff;
            yuv2rgb(r2[0], r2[1], r2[2], y[2 * j + src.linesize[0] + 0], u[j], v[j]); r2[3] = 0xff;
            yuv2rgb(r2[4], r2[5], r2[6], y[2 * j + src.linesize[0] + 1], u[j], v[j]); r2[7] = 0xff;
        }
        y += src.linesize[0] * 2;
        u += src.linesize[1];
        v += src.linesize[1];
    }
}

template <class T, class U, bool precise = true>
void NV12ToRGBA8(CVector<T> &dst, CVector<U> &src, size_t width, size_t height, CoefficientType type = CoefficientType::REC601)
{
    auto y = src.x, uv = src.y;
    auto dstptr = dst.x;

    auto linesize = width * 4;
    auto rows = width / 2;
    auto cols = height / 2;

    for (size_t i = 0; i < cols; i++, dstptr += linesize)
    {
        for (size_t j = 0; j < rows; j++, dstptr += 8)
        {
            auto r1 = dstptr;
            auto r2 = dstptr + linesize;
            auto u = uv[2 * j + 0];
            auto v = uv[2 * j + 1];
            yuv2rgb(r1[0], r1[1], r1[2], y[2 * j                   + 0], u, v); r1[3] = 0xff;
            yuv2rgb(r1[4], r1[5], r1[6], y[2 * j                   + 1], u, v); r1[7] = 0xff;
            yuv2rgb(r2[0], r2[1], r2[2], y[2 * j + src.linesize[0] + 0], u, v); r2[3] = 0xff;
            yuv2rgb(r2[4], r2[5], r2[6], y[2 * j + src.linesize[0] + 1], u, v); r2[7] = 0xff;
        }
        y  += src.linesize[0] * 2;
        uv += src.linesize[1];
    }
}

};

}
