#pragma once

#include <immintrin.h>
#include <iostream>

#include "slapi.h"
#include "slcast.h"

#ifndef __GNUC__
namespace sl
{
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

template <class T>
inline constexpr T load(const void *m)
{
    if constexpr (sl::typeof<T, __m128, __m128i, __m128d>())
    {
        return bcast<T>(_mm_load_ps(rcast<const float *>(m)));
    }
    if constexpr (sl::typeof<T, __m256, __m256i, __m256d>())
    {
        return bcast<T>(_mm256_load_ps(rcast<const float*>(m)));
    }
    if constexpr (sl::typeof<T, __m512, __m512i, __m512d>())
    {
        return bcast<T>(_mm512_load_ps(rcast<const float*>(m)));
    }
}

template <class T, class U>
inline constexpr T set(const U n)
{
    if constexpr (sl::typeof<U, INT64>())
    {
        if constexpr (sl::typeof<T, __m128, __m128i, __m128d>())
        {
            return bcast<T>(_mm_set1_epi64x(n));
        }
        if constexpr (sl::typeof<T, __m256, __m256i, __m256d>())
        {
            return bcast<T>(_mm256_set1_epi64x(n));
        }
        if constexpr (sl::typeof<T, __m512, __m512i, __m512d>())
        {
            return bcast<T>(_mm256_set1_epi64x(n));
        }
    }
    if constexpr (sl::typeof<U, double>())
    {
        if constexpr (sl::typeof<T, __m128, __m128i, __m128d>())
        {
            return bcast<T>(_mm_set1_pd(n));
        }
        if constexpr (sl::typeof<T, __m256, __m256i, __m256d>())
        {
            return bcast<T>(_mm256_set1_pd(n));
        }
        if constexpr (sl::typeof<T, __m512, __m512i, __m512d>())
        {
            return bcast<T>(_mm512_set1_pd(n));
        }
    }
    if constexpr (sl::typeof<U, INT32>())
    {
        if constexpr (sl::typeof<T, __m128, __m128i, __m128d>())
        {
            return bcast<T>(_mm_set1_epi32(n));
        }
        if constexpr (sl::typeof<T, __m256, __m256i, __m256d>())
        {
            return bcast<T>(_mm256_set1_epi32(n));
        }
        if constexpr (sl::typeof<T, __m512, __m512i, __m512d>())
        {
            return bcast<T>(_mm256_set1_epi32(n));
        }
    }
    if constexpr (sl::typeof<U, float>())
    {
        if constexpr (sl::typeof<T, __m128, __m128i, __m128d>())
        {
            return bcast<T>(_mm_set1_ps(n));
        }
        if constexpr (sl::typeof<T, __m256, __m256i, __m256d>())
        {
            return bcast<T>(_mm_set1_ps(n));
        }
        if constexpr (sl::typeof<T, __m512, __m512i, __m512d>())
        {
            return bcast<T>(_mm_set1_ps(n));
        }
    }
    if constexpr (sl::typeof<T, INT16>())
    {
        if constexpr (sl::typeof<T, __m128, __m128i, __m128d>())
        {
            return bcast<T>(_mm_set1_epi16(n));
        }
        if constexpr (sl::typeof<T, __m256, __m256i, __m256d>())
        {
            return bcast<T>(_mm256_set1_epi16(n));
        }
        if constexpr (sl::typeof<T, __m512, __m512i, __m512d>())
        {
            return bcast<T>(_mm256_set1_epi16(n));
        }
    }
    if constexpr (sl::typeof<T, INT8>())
    {
        if constexpr (sl::typeof<T, __m128, __m128i, __m128d>())
        {
            return bcast<T>(_mm_set1_epi8(n));
        }
        if constexpr (sl::typeof<T, __m256, __m256i, __m256d>())
        {
            return bcast<T>(_mm256_set1_epi8(n));
        }
        if constexpr (sl::typeof<T, __m512, __m512i, __m512d>())
        {
            return bcast<T>(_mm256_set1_epi8(n));
        }
    }
    return T();
}

template <class T>
inline constexpr T loadu(const void* m)
{
    if constexpr (sl::typeof<T, __m128, __m128i, __m128d>())
    {
        return bcast<T>(_mm_loadu_epi64(m));
    }
    if constexpr (sl::typeof<T, __m256, __m256i, __m256d>())
    {
        return bcast<T>(_mm256_loadu_epi64(m));
    }
    if constexpr (sl::typeof<T, __m512, __m512i, __m512d>())
    {
        return bcast<T>(_mm512_loadu_epi64(m));
    }
}

template <class T, class P>
inline constexpr void store(P dst, T src)
{
    if constexpr (sl::typeof<T, __m128, __m128i, __m128d>())
    {
        _mm_store_ps(rcast<float *>(dst), bcast<__m256>(src));
    }
    if constexpr (sl::typeof<T, __m256, __m256i, __m256d>())
    {
        _mm256_store_ps(rcast<float*>(dst), bcast<__m256>(src));
    }
    if constexpr (sl::typeof<T, __m512, __m512i, __m512d>())
    {
        _mm512_store_ps(rcast<float*>(dst), bcast<__m512>(src));
    }
}

template <class T, class P>
inline constexpr T storeu(P dst, T src)
{
    if constexpr (sl::typeof<T, __m128, __m128i, __m128d>())
    {
        _mm_storeu_epi64(dst, src);
    }
    if constexpr (sl::typeof<T, __m256, __m256i, __m256d>())
    {
        _mm256_storeu_epi64(dst, src);
    }
    if constexpr (sl::typeof<T, __m512, __m512i, __m512d>())
    {
        _mm512_storeu_epi64(dst, src);
    }
}

inline __m256i add(__m256i u, __m256i v)
{
    return _mm256_add_epi32(u, v);
}

inline __m256 add(__m256 u, __m256 v)
{
    return _mm256_add_ps(u, v);
}

inline __m256d add(__m256d u, __m256d v)
{
    return _mm256_add_pd(u, v);
}

inline __m256i mul(__m256i u, __m256i v)
{
    return _mm256_mullo_epi32(u, v);
}

inline __m256 mul(__m256 u, __m256 v)
{
    return _mm256_mul_ps(u, v);
}

inline __m256d mul(__m256d u, __m256d v)
{
    return _mm256_mul_pd(u, v);
}

template <class T, size_t index, class U>
inline constexpr T &Value(U u)
{
    return *(rcast<T*>(&u) + index);
}

template <class T, class P, class U>
inline constexpr T gather32(P ptr, U vindex, const int scale)
{
    if constexpr (sl::typeof<T, __m128, __m128i, __m128d>())
    {
        return _mm_i32gather_epi32(ptr, vindex, scale);
    }
    if constexpr (sl::typeof<T, __m256, __m256i, __m256d>())
    {
        return _mm256_i32gather_epi32(ptr, vindex, scale);
    }
    if constexpr (sl::typeof<T, __m512, __m512i, __m512d>())
    {
        return _mm512_i32gather_epi32(ptr, vindex, scale);
    }
}

template <class T, class P, class U>
inline constexpr T gather64(P ptr, U vindex, const int scale)
{
    if constexpr (sl::typeof<T, __m128, __m128i, __m128d>())
    {
        return _mm_i64gather_epi64(ptr, vindex, scale);
    }
    if constexpr (sl::typeof<T, __m256, __m256i, __m256d>())
    {
        return _mm256_i64gather_epi64(ptr, vindex, scale);
    }
    if constexpr (sl::typeof<T, __m512, __m512i, __m512d>())
    {
        return _mm512_i64gather_epi64(ptr, vindex, scale);
    }
}

template <class T>
inline constexpr T movehl(T h, T l)
{
    if constexpr (sl::typeof<T, __m128, __m128i, __m128d>())
    {
        return _mm_movehl_ps(bcast<__m128>(h), bcast<__m128>(l));
    }
    return T();
}

template <class T>
inline constexpr T movehdup(T h)
{
    if constexpr (sl::typeof<T, __m128, __m128i, __m128d>())
    {
        return _mm_movehdup_ps(bcast<__m128>(h));
    }
    return T();
}

template <class T>
inline constexpr T add(T a, T b)
{
if constexpr (sl::typeof<T, __m128>())
{
    return _mm_add_ps(a, b);
}
if constexpr (sl::typeof<T, __m128i>())
{
    return _mm_add_epi32(a, b);
}
if constexpr (sl::typeof<T, __m128d>())
{
    return _mm_add_pd(a, b);
}
if constexpr (sl::typeof<T, __m256>())
{
    return _mm256_add_ps(a, b);
}
if constexpr (sl::typeof<T, __m256i>())
{
    return _mm256_add_epi32(a, b);
}
if constexpr (sl::typeof<T, __m256d>())
{
    return _mm256_add_pd(a, b);
}
if constexpr (sl::typeof<T, __m512>())
{
    return _mm512_add_ps(a, b);
}
if constexpr (sl::typeof<T, __m512i>())
{
    return _mm512_add_epi32(a, b);
}
if constexpr (sl::typeof<T, __m512d>())
{
    return _mm512_add_pd(a, b);
}
return a + b;
}

template <class T>
inline constexpr T sub(T a, T b)
{
if constexpr (sl::typeof<T, __m128>)
{
    return _mm_sub_ps(a, b);
}
if constexpr (sl::typeof<T, __m128i>)
{
    return _mm_sub_epi32(a, b);
}
if constexpr (sl::typeof<T, __m128d>)
{
    return _mm_sub_pd(a, b);
}
if constexpr (sl::typeof<T, __m256>)
{
    return _mm256_sub_ps(a, b);
}
if constexpr (sl::typeof<T, __m256i>)
{
    return _mm256_sub_epi32(a, b);
}
if constexpr (sl::typeof<T, __m256d>)
{
    return _mm256_sub_pd(a, b);
}
if constexpr (sl::typeof<T, __m512>)
{
    return _mm512_sub_ps(a, b);
}
if constexpr (sl::typeof<T, __m512i>)
{
    return _mm512_sub_epi32(a, b);
}
if constexpr (sl::typeof<T, __m512d>)
{
    return _mm512_sub_pd(a, b);
}
return a - b;
}

template <class T>
inline constexpr T mul(T a, T b)
{
if constexpr (sl::typeof<T, __m128>())
{
    return _mm_mul_ps(a, b);
}
if constexpr (sl::typeof<T, __m128i>())
{
    return _mm_mullo_epi32(a, b);
}
if constexpr (sl::typeof<T, __m128d>())
{
    return _mm_mul_pd(a, b);
}
if constexpr (sl::typeof<T, __m256>())
{
    return _mm256_mul_ps(a, b);
}
if constexpr (sl::typeof<T, __m256i>())
{
    return _mm256_mullo_epi32(a, b);
}
if constexpr (sl::typeof<T, __m256d>())
{
    return _mm256_mul_pd(a, b);
}
if constexpr (sl::typeof<T, __m512>())
{
    return _mm512_mul_ps(a, b);
}
if constexpr (sl::typeof<T, __m512i>())
{
    return _mm512_mullo_epi32(a, b);
}
if constexpr (sl::typeof<T, __m512d>())
{
    return _mm512_mul_pd(a, b);
}
return a * b;
}
}

static inline std::ostream& operator<<(std::ostream& o, __m256i v)
{
printf("%4d%4d%4d%4d%4d%4d%4d%4d",
    sl::Value<int, 0>(v),
    sl::Value<int, 1>(v),
    sl::Value<int, 2>(v),
    sl::Value<int, 3>(v),
    sl::Value<int, 4>(v),
    sl::Value<int, 5>(v),
    sl::Value<int, 6>(v),
    sl::Value<int, 7>(v)
);

return o;
}
#endif
