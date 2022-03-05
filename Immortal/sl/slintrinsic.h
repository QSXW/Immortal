#pragma once

#ifndef SLINTRINSIC_H__
#define SLINTRINSIC_H__

#include <immintrin.h>
#include <iostream>

#include "slapi.h"
#include "slcast.h"

namespace sl
{

#define CONSTRUCTOR_PRIMITIVE() R(Primitive v) : v{ v } {} operator Primitive() const { return v; };
#define CONSTURCTOR_SET1(P, S, T) R(T n) : v{ _m##P##_set1_##S(n) } {}
#define CONSTURCTOR_LOAD(P, S, T) R(const void *data) : v{ _m##P##_load_##S((T *)data) } {}

#define DEFINE_OPERATOR(P, N, O, T) R operator N (R &b) { R p{ _m##P##_##O##_##T(v, b.v) }; return p; }
#define OPEARTOR_MUL(P, T) DEFINE_OPERATOR(P, *, mul, T)
#define OPEARTOR_ADD(P, T) DEFINE_OPERATOR(P, +, add, T)
#define OPEARTOR_SUB(P, T) DEFINE_OPERATOR(P, -, sub, T)

#define DEFINE_ALIGNED_STORE(P, S)   void aligned_store(void *dst) { _m##P##_store_##S(dst, v); }
#define DEFINE_UNALIGNED_STORE(P, S) void store(void *dst) { _m##P##_storeu_##S(dst, v); }

struct int32x4
{
#define R int32x4
public:
    using Primitive = __m128i;

public:
    CONSTRUCTOR_PRIMITIVE()
    CONSTURCTOR_SET1(m, epi8,   int8_t)
    CONSTURCTOR_SET1(m, epi16,  int16_t)
    CONSTURCTOR_SET1(m, epi32,  int32_t)
    CONSTURCTOR_SET1(m, epi64x, int64_t)
    CONSTURCTOR_LOAD(m, si128,  Primitive)

    OPEARTOR_ADD(m, epi32)
    OPEARTOR_SUB(m, epi32)
    OPEARTOR_MUL(m, epi32)

#undef R
private:
    Primitive v;
};

struct floatx4
{
#define R floatx4
public:
    using Primitive = __m128;

public:
    floatx4() = default;

    CONSTRUCTOR_PRIMITIVE()
    CONSTURCTOR_SET1(m, ps, float)
    CONSTURCTOR_LOAD(m, ps, float)

    OPEARTOR_ADD(m, ps)
    OPEARTOR_SUB(m, ps)
    OPEARTOR_MUL(m, ps)

    operator float() const
    {
        return *((float*)&v);
    }

    floatx4 movehl(floatx4 &other)
    {
        return _mm_movehl_ps(v, other.v);
    }

    floatx4 movehdup() const
    {
        return _mm_movehdup_ps(v);
    }

#undef R
private:
    Primitive v;
};

struct int32x8
{
#define R int32x8
public:
    using Primitive = __m256i;

public:
    CONSTRUCTOR_PRIMITIVE()
    CONSTURCTOR_SET1(m256, epi8,   int8_t)
    CONSTURCTOR_SET1(m256, epi16,  int16_t)
    CONSTURCTOR_SET1(m256, epi32,  int32_t)
    CONSTURCTOR_SET1(m256, epi64x, int64_t)
    CONSTURCTOR_LOAD(m256, si256,  Primitive)

    OPEARTOR_ADD(m256, epi32)
    OPEARTOR_SUB(m256, epi32)
    OPEARTOR_MUL(m256, epi32)

#undef R
private:
    Primitive v;
};

struct floatx8
{
#define R floatx8
public:
    using Primitive = __m256;

public:
    floatx8() = default;

    CONSTRUCTOR_PRIMITIVE()
    CONSTURCTOR_SET1(m256, ps, float)
    CONSTURCTOR_LOAD(m256, ps, float)

    OPEARTOR_ADD(m256, ps)
    OPEARTOR_SUB(m256, ps)
    OPEARTOR_MUL(m256, ps)

#undef R
private:
    Primitive v;
};

struct int32x16
{
#define R int32x16
public:
    using Primitive = __m512i;

public:
    CONSTRUCTOR_PRIMITIVE()
    CONSTURCTOR_SET1(m512, epi8,  int8_t)
    CONSTURCTOR_SET1(m512, epi16, int16_t)
    CONSTURCTOR_SET1(m512, epi32, int32_t)
    CONSTURCTOR_SET1(m512, epi64, int64_t)
    CONSTURCTOR_LOAD(m512, si512, Primitive)

    OPEARTOR_ADD(m512, epi32)
    OPEARTOR_SUB(m512, epi32)
    OPEARTOR_MUL(m512, epi32)

#undef R
private:
    Primitive v;
};

struct floatx16
{
#define R floatx16
public:
    using Primitive = __m512;

public:
    floatx16() = default;

    CONSTRUCTOR_PRIMITIVE()
    CONSTURCTOR_SET1(m512, ps, float)
    CONSTURCTOR_LOAD(m512, ps, float)

    OPEARTOR_ADD(m512, ps)
    OPEARTOR_SUB(m512, ps)
    OPEARTOR_MUL(m512, ps)

    Primitive permutexvar(const int32x16 &vindex)
    {
       return _mm512_permutexvar_ps(vindex, v);
    }

#undef R
private:
    Primitive v;
};

struct int16x32
{
#define R int16x32
public:
    using Primitive = __m512i;

public:
    CONSTRUCTOR_PRIMITIVE()
    CONSTURCTOR_SET1(m512, epi8,  int8_t)
    CONSTURCTOR_SET1(m512, epi16, int16_t)
    CONSTURCTOR_SET1(m512, epi32, int32_t)
    CONSTURCTOR_SET1(m512, epi64, int64_t)
    CONSTURCTOR_LOAD(m512, si512, Primitive)

    OPEARTOR_ADD(m512, epi16)
    OPEARTOR_SUB(m512, epi16)
    DEFINE_OPERATOR(m512, *, mullo, epi16)

    DEFINE_ALIGNED_STORE(m512, epi64)
    DEFINE_UNALIGNED_STORE(m512, epi64)

#undef R
private:
    Primitive v;
};

}

#endif /* SLINTRINSIC_H__ */
