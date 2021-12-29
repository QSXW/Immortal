#pragma once

#ifndef __SLINTRINSIC_H__
#define __SLINTRINSIC_H__

#include <immintrin.h>
#include <iostream>

#include "slapi.h"
#include "slcast.h"

using namespace sl;

#ifndef __GNUC__

#define SL_DONT_USE_DIV [[deprecated("avoid to use, try convert to a multiply before broadcast")]]

using m32 = uint32_t;
using m64 = uint64_t;

using m128 = __m128;
using m256 = __m256;
using m512 = __m512;

using m128i = __m128i;
using m256i = __m256i;
using m512i = __m512i;

using m128d = __m128d;
using m256d = __m256d;
using m512d = __m512d;

template <class T, class U>
inline constexpr T load(const U *m)
{
    if constexpr (sl::IsPrimitiveOf<T, m128, m128i, m128d>())
    {
        return bcast<T>();
    }
    if constexpr (sl::IsPrimitiveOf<T, m256, m256i, m256d>())
    {
        return bcast<T>(_mm256_load_ps(rcast<const float*>(m)));
    }
    if constexpr (sl::IsPrimitiveOf<T, m512, m512i, m512d>())
    {
        return bcast<T>(_mm512_load_ps(rcast<const float*>(m)));
    }
}

template <class T, class U>
inline constexpr T set(const U n)
{
    if constexpr (sl::IsPrimitiveOf<U, m64>())
    {
        if constexpr (sl::IsPrimitiveOf<T, __m128, __m128i, __m128d>())
        {
            return bcast<T>(_mm_set1_epi64x(n));
        }
        if constexpr (sl::IsPrimitiveOf<T, __m256, __m256i, __m256d>())
        {
            return bcast<T>(_mm256_set1_epi64x(n));
        }
        if constexpr (sl::IsPrimitiveOf<T, __m512, __m512i, __m512d>())
        {
            return bcast<T>(_mm512_set1_epi64x(n));
        }
    }
    if constexpr (sl::IsPrimitiveOf<U, double>())
    {
        if constexpr (sl::IsPrimitiveOf<T, __m128, __m128i, __m128d>())
        {
            return bcast<T>(_mm_set1_pd(n));
        }
        if constexpr (sl::IsPrimitiveOf<T, __m256, __m256i, __m256d>())
        {
            return bcast<T>(_mm256_set1_pd(n));
        }
        if constexpr (sl::IsPrimitiveOf<T, __m512, __m512i, __m512d>())
        {
            return bcast<T>(_mm512_set1_pd(n));
        }
    }
    if constexpr (sl::IsPrimitiveOf<U, m32>())
    {
        if constexpr (sl::IsPrimitiveOf<T, __m128, __m128i, __m128d>())
        {
            return bcast<T>(_mm_set1_epi32(n));
        }
        if constexpr (sl::IsPrimitiveOf<T, __m256, __m256i, __m256d>())
        {
            return bcast<T>(_mm256_set1_epi32(n));
        }
        if constexpr (sl::IsPrimitiveOf<T, __m512, __m512i, __m512d>())
        {
            return bcast<T>(_mm512_set1_epi32(n));
        }
    }
    if constexpr (sl::IsPrimitiveOf<U, float>())
    {
        if constexpr (sl::IsPrimitiveOf<T, __m128, __m128i, __m128d>())
        {
            return bcast<T>(_mm_set1_ps(n));
        }
        if constexpr (sl::IsPrimitiveOf<T, __m256, __m256i, __m256d>())
        {
            return bcast<T>(_mm256_set1_ps(n));
        }
        if constexpr (sl::IsPrimitiveOf<T, __m512, __m512i, __m512d>())
        {
            return bcast<T>(_mm512_set1_ps(n));
        }
    }
    if constexpr (sl::IsPrimitiveOf<T, unsigned short>())
    {
        if constexpr (sl::IsPrimitiveOf<T, __m128, __m128i, __m128d>())
        {
            return bcast<T>(_mm_set1_epi16(n));
        }
        if constexpr (sl::IsPrimitiveOf<T, __m256, __m256i, __m256d>())
        {
            return bcast<T>(_mm256_set1_epi16(n));
        }
        if constexpr (sl::IsPrimitiveOf<T, __m512, __m512i, __m512d>())
        {
            return bcast<T>(_mm512_set1_epi16(n));
        }
    }
    if constexpr (sl::IsPrimitiveOf<T, unsigned char>())
    {
        if constexpr (sl::IsPrimitiveOf<T, __m128, __m128i, __m128d>())
        {
            return bcast<T>(_mm_set1_epi8(n));
        }
        if constexpr (sl::IsPrimitiveOf<T, __m256, __m256i, __m256d>())
        {
            return bcast<T>(_mm256_set1_epi8(n));
        }
        if constexpr (sl::IsPrimitiveOf<T, __m512, __m512i, __m512d>())
        {
            return bcast<T>(_mm256_set1_epi8(n));
        }
    }
    return T();
}

template <class T>
inline constexpr T loadu(const void* m)
{
    if constexpr (sl::IsPrimitiveOf<T, __m128, __m128i, __m128d>())
    {
        return bcast<T>(_mm_loadu_epi64(m));
    }
    if constexpr (sl::IsPrimitiveOf<T, __m256, __m256i, __m256d>())
    {
        return bcast<T>(_mm256_loadu_epi64(m));
    }
    if constexpr (sl::IsPrimitiveOf<T, __m512, __m512i, __m512d>())
    {
        return bcast<T>(_mm512_loadu_epi64(m));
    }
}

template <class T, class P>
inline constexpr void store(P dst, T src)
{
    if constexpr (sl::IsPrimitiveOf<T, __m128, __m128i, __m128d>())
    {
        _mm_store_ps(rcast<float *>(dst), bcast<__m256>(src));
    }
    if constexpr (sl::IsPrimitiveOf<T, __m256, __m256i, __m256d>())
    {
        _mm256_store_ps(rcast<float*>(dst), bcast<__m256>(src));
    }
    if constexpr (sl::IsPrimitiveOf<T, __m512, __m512i, __m512d>())
    {
        _mm512_store_ps(rcast<float*>(dst), bcast<__m512>(src));
    }
}

template <class T, class P>
inline constexpr T storeu(P dst, T src)
{
    if constexpr (sl::IsPrimitiveOf<T, __m128, __m128i, __m128d>())
    {
        _mm_storeu_epi64(dst, src);
    }
    if constexpr (sl::IsPrimitiveOf<T, __m256, __m256i, __m256d>())
    {
        _mm256_storeu_epi64(dst, src);
    }
    if constexpr (sl::IsPrimitiveOf<T, __m512, __m512i, __m512d>())
    {
        _mm512_storeu_epi64(dst, src);
    }
}

template <class T, size_t index, class U>
inline constexpr T &Value(U u)
{
    return *(rcast<T*>(&u) + index);
}

template <class T, class P, class U>
inline constexpr T gather32(P ptr, U vindex, const int scale)
{
    if constexpr (sl::IsPrimitiveOf<T, __m128, __m128i, __m128d>())
    {
        return _mm_i32gather_epi32(ptr, vindex, scale);
    }
    if constexpr (sl::IsPrimitiveOf<T, __m256, __m256i, __m256d>())
    {
        return _mm256_i32gather_epi32(ptr, vindex, scale);
    }
    if constexpr (sl::IsPrimitiveOf<T, __m512, __m512i, __m512d>())
    {
        return _mm512_i32gather_epi32(ptr, vindex, scale);
    }
}

template <class T, class P, class U>
inline constexpr T gather64(P ptr, U vindex, const int scale)
{
    if constexpr (sl::IsPrimitiveOf<T, __m128, __m128i, __m128d>())
    {
        return _mm_i64gather_epi64(ptr, vindex, scale);
    }
    if constexpr (sl::IsPrimitiveOf<T, __m256, __m256i, __m256d>())
    {
        return _mm256_i64gather_epi64(ptr, vindex, scale);
    }
    if constexpr (sl::IsPrimitiveOf<T, __m512, __m512i, __m512d>())
    {
        return _mm512_i64gather_epi64(ptr, vindex, scale);
    }
}

template <class T>
inline constexpr T movehl(T h, T l)
{
    if constexpr (sl::IsPrimitiveOf<T, __m128, __m128i, __m128d>())
    {
        return _mm_movehl_ps(bcast<__m128>(h), bcast<__m128>(l));
    }
    return T();
}

template <class T>
inline constexpr T movehdup(T h)
{
    if constexpr (sl::IsPrimitiveOf<T, __m128, __m128i, __m128d>())
    {
        return _mm_movehdup_ps(bcast<__m128>(h));
    }
    return T();
}

inline m128 operator+(m128 a, m128 b)
{
    return _mm_add_ps(a, b);
}

inline m128i operator+(m128i a, m128i b)
{
    return _mm_add_epi32(a, b);
}

inline m128d operator+(m128d a, m128d b)
{
    return _mm_add_pd(a, b);
}

inline m256 operator+(m256 a, m256 b)
{
    return _mm256_add_ps(a, b);
}

inline m256i operator+(m256i a, m256i b)
{
    return _mm256_add_epi32(a, b);
}

inline m256d operator+(m256d a, m256d b)
{
    return _mm256_add_pd(a, b);
}

inline m512 operator+(m512 a, m512 b)
{
    return _mm512_add_ps(a, b);
}

inline m512i operator+(m512i a, m512i b)
{
    return _mm512_add_epi32(a, b);
}

inline m512d operator+(m512d a, m512d b)
{
    return _mm512_add_pd(a, b);
}

inline m128 operator-(m128 a, m128 b)
{
    return _mm_sub_ps(a, b);
}

inline m128i operator-(m128i a, m128i b)
{
    return _mm_sub_epi32(a, b);
}

inline m128d operator-(m128d a, m128d b)
{
    return _mm_sub_pd(a, b);
}

inline m256 operator-(m256 a, m256 b)
{
    return _mm256_sub_ps(a, b);
}

inline m256i operator-(m256i a, m256i b)
{
    return _mm256_sub_epi32(a, b);
}

inline m256d operator-(m256d a, m256d b)
{
    return _mm256_sub_pd(a, b);
}

inline m512 operator-(m512 a, m512 b)
{
    return _mm512_sub_ps(a, b);
}

inline m512i operator-(m512i a, m512i b)
{
    return _mm512_sub_epi32(a, b);
}

inline m512d operator-(m512d a, m512d b)
{
    return _mm512_sub_pd(a, b);
}

inline m128 operator*(m128 a, m128 b)
{
    return _mm_mul_ps(a, b);
}

inline m128i operator*(m128i a, m128i b)
{
    return _mm_mul_epi32(a, b);
}

inline m128d operator*(m128d a, m128d b)
{
    return _mm_mul_pd(a, b);
}

inline m256 operator*(m256 a, m256 b)
{
    return _mm256_mul_ps(a, b);
}

inline m256i operator*(m256i a, m256i b)
{
    return _mm256_mul_epi32(a, b);
}

inline m256d operator*(m256d a, m256d b)
{
    return _mm256_mul_pd(a, b);
}

inline m512 operator*(m512 a, m512 b)
{
    return _mm512_mul_ps(a, b);
}

inline m512i operator*(m512i a, m512i b)
{
    return _mm512_mul_epi32(a, b);
}

inline m512d operator*(m512d a, m512d b)
{
    return _mm512_mul_pd(a, b);
}

inline SL_DONT_USE_DIV m128 operator/(m128 a, m128 b)
{
    return _mm_div_ps(a, b);
}


inline SL_DONT_USE_DIV m128i operator/(m128i a, m128i b)
{
    return _mm_div_epi32(a, b);
}

inline SL_DONT_USE_DIV m128d operator/(m128d a, m128d b)
{
    return _mm_div_pd(a, b);
}

inline SL_DONT_USE_DIV m256 operator/(m256 a, m256 b)
{
    return _mm256_div_ps(a, b);
}

inline  m256i operator/ SL_DONT_USE_DIV (m256i a, m256i b)
{
    return _mm256_div_epi32(a, b);
}

inline SL_DONT_USE_DIV m256d operator/(m256d a, m256d b)
{
    return _mm256_div_pd(a, b);
}

inline SL_DONT_USE_DIV m512 operator/(m512 a, m512 b)
{
    return _mm512_div_ps(a, b);
}

inline SL_DONT_USE_DIV m512i operator/(m512i a, m512i b)
{
    return _mm512_div_epi32(a, b);
}

inline SL_DONT_USE_DIV m512d operator/(m512d a, m512d b)
{
    return _mm512_div_pd(a, b);
}

static inline std::ostream& operator<<(std::ostream& o, __m256i v)
{
    printf("%4d%4d%4d%4d%4d%4d%4d%4d",
        Value<int, 0>(v),
        Value<int, 1>(v),
        Value<int, 2>(v),
        Value<int, 3>(v),
        Value<int, 4>(v),
        Value<int, 5>(v),
        Value<int, 6>(v),
        Value<int, 7>(v)
        );

    return o;
}

#endif /* __GNUC__ */
#endif /* __SLINTRINSIC_H__ */
