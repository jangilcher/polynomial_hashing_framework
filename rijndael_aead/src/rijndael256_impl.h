#pragma once
#include "rijndael256/rijndael256.h"
#include <immintrin.h>
#include <inttypes.h>

inline void rijndael256block(__m128i out[2], const __m128i roundkeys[30],
                             const __m128i in[2])
{
    __m128i key[2];
    __m128i tmp_in[2];
    __m128i shuffle = shufmask.v;
    __m128i blend = blendmask.v;
    __m128i input[2];
    input[0] = _mm_xor_si128(in[0], roundkeys[0]);
    input[1] = _mm_xor_si128(in[1], roundkeys[1]);

#ifdef __GNUC__
#ifdef __clang__
#pragma unroll 14
#else
#pragma GCC unroll 14
#endif
#endif
    for (int i = 1; i < 14; i++) {
        tmp_in[0] = _mm_blendv_epi8(input[1], input[0], blend);
        tmp_in[1] = _mm_blendv_epi8(input[0], input[1], blend);
        key[0] = roundkeys[2 * i];
        key[1] = roundkeys[2 * i + 1];
        input[0] = _mm_shuffle_epi8(tmp_in[0], shuffle);
        input[1] = _mm_shuffle_epi8(tmp_in[1], shuffle);
        input[0] = _mm_aesenc_si128(input[0], key[0]);
        input[1] = _mm_aesenc_si128(input[1], key[1]);
    }
    tmp_in[0] = _mm_blendv_epi8(input[1], input[0], blend);
    tmp_in[1] = _mm_blendv_epi8(input[0], input[1], blend);
    key[0] = roundkeys[28];
    key[1] = roundkeys[29];
    input[0] = _mm_shuffle_epi8(tmp_in[0], shuffle);
    input[1] = _mm_shuffle_epi8(tmp_in[1], shuffle);
    out[0] = _mm_aesenclast_si128(input[0], key[0]);
    out[1] = _mm_aesenclast_si128(input[1], key[1]);
    return;
}

static inline void rijndael256blockXOR(__m128i out[2],
                                       const __m128i roundkeys[30],
                                       const __m128i in[2],
                                       const __m128i xor [2])
{
    __m128i key[2];
    __m128i tmp_in[2];
    __m128i shuffle = shufmask.v;
    __m128i blend = blendmask.v;
    __m128i input[2];
    input[0] = _mm_xor_si128(in[0], roundkeys[0]);
    input[1] = _mm_xor_si128(in[1], roundkeys[1]);
#ifdef __GNUC__
#ifdef __clang__
#pragma unroll 14
#else
#pragma GCC unroll 14
#endif
#endif
    for (int i = 1; i < 14; i++) {
        tmp_in[0] = _mm_blendv_epi8(input[1], input[0], blend);
        tmp_in[1] = _mm_blendv_epi8(input[0], input[1], blend);
        key[0] = roundkeys[2 * i];
        key[1] = roundkeys[2 * i + 1];
        input[0] = _mm_shuffle_epi8(tmp_in[0], shuffle);
        input[1] = _mm_shuffle_epi8(tmp_in[1], shuffle);
        input[0] = _mm_aesenc_si128(input[0], key[0]);
        input[1] = _mm_aesenc_si128(input[1], key[1]);
    }
    tmp_in[0] = _mm_blendv_epi8(input[1], input[0], blend);
    tmp_in[1] = _mm_blendv_epi8(input[0], input[1], blend);
    key[0] = _mm_xor_si128(xor[0], roundkeys[28]);
    key[1] = _mm_xor_si128(xor[1], roundkeys[29]);
    input[0] = _mm_shuffle_epi8(tmp_in[0], shuffle);
    input[1] = _mm_shuffle_epi8(tmp_in[1], shuffle);
    out[0] = _mm_aesenclast_si128(input[0], key[0]);
    out[1] = _mm_aesenclast_si128(input[1], key[1]);
    return;
}

static inline void rijndael256blockXORx2(__m128i out[2*2],
                                       const __m128i roundkeys[30],
                                       const __m128i in[2*2],
                                       const __m128i xor[2*2])
{
    __m128i key[2*2];
    __m128i tmp_in[2*2];
    __m128i shuffle = shufmask.v;
    __m128i blend = blendmask.v;
    __m128i input[2*2];
    input[0*2+0] = _mm_xor_si128(in[0*2+0], roundkeys[0]);
    input[1*2+0] = _mm_xor_si128(in[1*2+0], roundkeys[0]);
    input[0*2+1] = _mm_xor_si128(in[0*2+1], roundkeys[1]);
    input[1*2+1] = _mm_xor_si128(in[1*2+1], roundkeys[1]);
#ifdef __GNUC__
#ifdef __clang__
#pragma unroll 14
#else
#pragma GCC unroll 14
#endif
#endif
    for (int i = 1; i < 14; i++) {
        tmp_in[0*2+0] = _mm_blendv_epi8(input[0*2+1], input[0*2+0], blend);
        tmp_in[1*2+0] = _mm_blendv_epi8(input[1*2+1], input[1*2+0], blend);
        tmp_in[0*2+1] = _mm_blendv_epi8(input[0*2+0], input[0*2+1], blend);
        tmp_in[1*2+1] = _mm_blendv_epi8(input[1*2+0], input[1*2+1], blend);
        key[0] = roundkeys[2 * i];
        key[1] = roundkeys[2 * i + 1];
        input[0*2+0] = _mm_shuffle_epi8(tmp_in[0*2+0], shuffle);
        input[1*2+0] = _mm_shuffle_epi8(tmp_in[1*2+0], shuffle);
        input[0*2+1] = _mm_shuffle_epi8(tmp_in[0*2+1], shuffle);
        input[1*2+1] = _mm_shuffle_epi8(tmp_in[1*2+1], shuffle);
        input[0*2+0] = _mm_aesenc_si128(input[0*2+0], key[0]);
        input[1*2+0] = _mm_aesenc_si128(input[1*2+0], key[0]);
        input[0*2+1] = _mm_aesenc_si128(input[0*2+1], key[1]);
        input[1*2+1] = _mm_aesenc_si128(input[1*2+1], key[1]);
    }
    tmp_in[0*2+0] = _mm_blendv_epi8(input[0*2+1], input[0*2+0], blend);
    tmp_in[1*2+0] = _mm_blendv_epi8(input[1*2+1], input[1*2+0], blend);
    tmp_in[0*2+1] = _mm_blendv_epi8(input[0*2+0], input[0*2+1], blend);
    tmp_in[1*2+1] = _mm_blendv_epi8(input[1*2+0], input[1*2+1], blend);
    key[0*2+0] = _mm_xor_si128(xor[0*2+0], roundkeys[28]);
    key[1*2+0] = _mm_xor_si128(xor[1*2+0], roundkeys[28]);
    key[0*2+1] = _mm_xor_si128(xor[0*2+1], roundkeys[29]);
    key[1*2+1] = _mm_xor_si128(xor[1*2+1], roundkeys[29]);
    input[0*2+0] = _mm_shuffle_epi8(tmp_in[0*2+0], shuffle);
    input[1*2+0] = _mm_shuffle_epi8(tmp_in[1*2+0], shuffle);
    input[0*2+1] = _mm_shuffle_epi8(tmp_in[0*2+1], shuffle);
    input[1*2+1] = _mm_shuffle_epi8(tmp_in[1*2+1], shuffle);
    out[0*2+0] = _mm_aesenclast_si128(input[0*2+0], key[0*2+0]);
    out[1*2+0] = _mm_aesenclast_si128(input[1*2+0], key[1*2+0]);
    out[0*2+1] = _mm_aesenclast_si128(input[0*2+1], key[0*2+1]);
    out[1*2+1] = _mm_aesenclast_si128(input[1*2+1], key[1*2+1]);
    return;
}

static inline void rijndael256blockXORx3(__m128i out[3*2],
                                       const __m128i roundkeys[30],
                                       const __m128i in[3*2],
                                       const __m128i xor[3*2])
{
    __m128i key[3*2];
    __m128i tmp_in[3*2];
    __m128i shuffle = shufmask.v;
    __m128i blend = blendmask.v;
    __m128i input[3*2];
    input[0*2+0] = _mm_xor_si128(in[0*2+0], roundkeys[0]);
    input[1*2+0] = _mm_xor_si128(in[1*2+0], roundkeys[0]);
    input[2*2+0] = _mm_xor_si128(in[2*2+0], roundkeys[0]);
    input[0*2+1] = _mm_xor_si128(in[0*2+1], roundkeys[1]);
    input[1*2+1] = _mm_xor_si128(in[1*2+1], roundkeys[1]);
    input[2*2+1] = _mm_xor_si128(in[2*2+1], roundkeys[1]);
#ifdef __GNUC__
#ifdef __clang__
#pragma unroll 14
#else
#pragma GCC unroll 14
#endif
#endif
    for (int i = 1; i < 14; i++) {
        tmp_in[0*2+0] = _mm_blendv_epi8(input[0*2+1], input[0*2+0], blend);
        tmp_in[1*2+0] = _mm_blendv_epi8(input[1*2+1], input[1*2+0], blend);
        tmp_in[2*2+0] = _mm_blendv_epi8(input[2*2+1], input[2*2+0], blend);
        tmp_in[0*2+1] = _mm_blendv_epi8(input[0*2+0], input[0*2+1], blend);
        tmp_in[1*2+1] = _mm_blendv_epi8(input[1*2+0], input[1*2+1], blend);
        tmp_in[2*2+1] = _mm_blendv_epi8(input[2*2+0], input[2*2+1], blend);
        key[0] = roundkeys[2 * i];
        key[1] = roundkeys[2 * i + 1];
        input[0*2+0] = _mm_shuffle_epi8(tmp_in[0*2+0], shuffle);
        input[1*2+0] = _mm_shuffle_epi8(tmp_in[1*2+0], shuffle);
        input[2*2+0] = _mm_shuffle_epi8(tmp_in[2*2+0], shuffle);
        input[0*2+1] = _mm_shuffle_epi8(tmp_in[0*2+1], shuffle);
        input[1*2+1] = _mm_shuffle_epi8(tmp_in[1*2+1], shuffle);
        input[2*2+1] = _mm_shuffle_epi8(tmp_in[2*2+1], shuffle);
        input[0*2+0] = _mm_aesenc_si128(input[0*2+0], key[0]);
        input[1*2+0] = _mm_aesenc_si128(input[1*2+0], key[0]);
        input[2*2+0] = _mm_aesenc_si128(input[2*2+0], key[0]);
        input[0*2+1] = _mm_aesenc_si128(input[0*2+1], key[1]);
        input[1*2+1] = _mm_aesenc_si128(input[1*2+1], key[1]);
        input[2*2+1] = _mm_aesenc_si128(input[2*2+1], key[1]);
    }
    tmp_in[0*2+0] = _mm_blendv_epi8(input[0*2+1], input[0*2+0], blend);
    tmp_in[1*2+0] = _mm_blendv_epi8(input[1*2+1], input[1*2+0], blend);
    tmp_in[2*2+0] = _mm_blendv_epi8(input[2*2+1], input[2*2+0], blend);
    tmp_in[0*2+1] = _mm_blendv_epi8(input[0*2+0], input[0*2+1], blend);
    tmp_in[1*2+1] = _mm_blendv_epi8(input[1*2+0], input[1*2+1], blend);
    tmp_in[2*2+1] = _mm_blendv_epi8(input[2*2+0], input[2*2+1], blend);
    key[0*2+0] = _mm_xor_si128(xor[0*2+0], roundkeys[28]);
    key[1*2+0] = _mm_xor_si128(xor[1*2+0], roundkeys[28]);
    key[2*2+0] = _mm_xor_si128(xor[2*2+0], roundkeys[28]);
    key[0*2+1] = _mm_xor_si128(xor[0*2+1], roundkeys[29]);
    key[1*2+1] = _mm_xor_si128(xor[1*2+1], roundkeys[29]);
    key[2*2+1] = _mm_xor_si128(xor[2*2+1], roundkeys[29]);
    input[0*2+0] = _mm_shuffle_epi8(tmp_in[0*2+0], shuffle);
    input[1*2+0] = _mm_shuffle_epi8(tmp_in[1*2+0], shuffle);
    input[2*2+0] = _mm_shuffle_epi8(tmp_in[2*2+0], shuffle);
    input[0*2+1] = _mm_shuffle_epi8(tmp_in[0*2+1], shuffle);
    input[1*2+1] = _mm_shuffle_epi8(tmp_in[1*2+1], shuffle);
    input[2*2+1] = _mm_shuffle_epi8(tmp_in[2*2+1], shuffle);
    out[0*2+0] = _mm_aesenclast_si128(input[0*2+0], key[0*2+0]);
    out[1*2+0] = _mm_aesenclast_si128(input[1*2+0], key[1*2+0]);
    out[2*2+0] = _mm_aesenclast_si128(input[2*2+0], key[2*2+0]);
    out[0*2+1] = _mm_aesenclast_si128(input[0*2+1], key[0*2+1]);
    out[1*2+1] = _mm_aesenclast_si128(input[1*2+1], key[1*2+1]);
    out[2*2+1] = _mm_aesenclast_si128(input[2*2+1], key[2*2+1]);
    return;
}

static inline void rijndael256blockXORx4(__m128i out[4*2],
                                       const __m128i roundkeys[30],
                                       const __m128i in[4*2],
                                       const __m128i xor[4*2])
{
    __m128i key[4*2];
    __m128i tmp_in[4*2];
    __m128i shuffle = shufmask.v;
    __m128i blend = blendmask.v;
    __m128i input[4*2];
    input[0*2+0] = _mm_xor_si128(in[0*2+0], roundkeys[0]);
    input[1*2+0] = _mm_xor_si128(in[1*2+0], roundkeys[0]);
    input[2*2+0] = _mm_xor_si128(in[2*2+0], roundkeys[0]);
    input[3*2+0] = _mm_xor_si128(in[3*2+0], roundkeys[0]);
    input[0*2+1] = _mm_xor_si128(in[0*2+1], roundkeys[1]);
    input[1*2+1] = _mm_xor_si128(in[1*2+1], roundkeys[1]);
    input[2*2+1] = _mm_xor_si128(in[2*2+1], roundkeys[1]);
    input[3*2+1] = _mm_xor_si128(in[3*2+1], roundkeys[1]);
#ifdef __GNUC__
#ifdef __clang__
#pragma unroll 14
#else
#pragma GCC unroll 14
#endif
#endif
    for (int i = 1; i < 14; i++) {
        tmp_in[0*2+0] = _mm_blendv_epi8(input[0*2+1], input[0*2+0], blend);
        tmp_in[1*2+0] = _mm_blendv_epi8(input[1*2+1], input[1*2+0], blend);
        tmp_in[2*2+0] = _mm_blendv_epi8(input[2*2+1], input[2*2+0], blend);
        tmp_in[3*2+0] = _mm_blendv_epi8(input[3*2+1], input[3*2+0], blend);
        tmp_in[0*2+1] = _mm_blendv_epi8(input[0*2+0], input[0*2+1], blend);
        tmp_in[1*2+1] = _mm_blendv_epi8(input[1*2+0], input[1*2+1], blend);
        tmp_in[2*2+1] = _mm_blendv_epi8(input[2*2+0], input[2*2+1], blend);
        tmp_in[3*2+1] = _mm_blendv_epi8(input[3*2+0], input[3*2+1], blend);
        key[0] = roundkeys[2 * i];
        key[1] = roundkeys[2 * i + 1];
        input[0*2+0] = _mm_shuffle_epi8(tmp_in[0*2+0], shuffle);
        input[1*2+0] = _mm_shuffle_epi8(tmp_in[1*2+0], shuffle);
        input[2*2+0] = _mm_shuffle_epi8(tmp_in[2*2+0], shuffle);
        input[3*2+0] = _mm_shuffle_epi8(tmp_in[3*2+0], shuffle);
        input[0*2+1] = _mm_shuffle_epi8(tmp_in[0*2+1], shuffle);
        input[1*2+1] = _mm_shuffle_epi8(tmp_in[1*2+1], shuffle);
        input[2*2+1] = _mm_shuffle_epi8(tmp_in[2*2+1], shuffle);
        input[3*2+1] = _mm_shuffle_epi8(tmp_in[3*2+1], shuffle);
        input[0*2+0] = _mm_aesenc_si128(input[0*2+0], key[0]);
        input[1*2+0] = _mm_aesenc_si128(input[1*2+0], key[0]);
        input[2*2+0] = _mm_aesenc_si128(input[2*2+0], key[0]);
        input[3*2+0] = _mm_aesenc_si128(input[3*2+0], key[0]);
        input[0*2+1] = _mm_aesenc_si128(input[0*2+1], key[1]);
        input[1*2+1] = _mm_aesenc_si128(input[1*2+1], key[1]);
        input[2*2+1] = _mm_aesenc_si128(input[2*2+1], key[1]);
        input[3*2+1] = _mm_aesenc_si128(input[3*2+1], key[1]);
    }
    tmp_in[0*2+0] = _mm_blendv_epi8(input[0*2+1], input[0*2+0], blend);
    tmp_in[1*2+0] = _mm_blendv_epi8(input[1*2+1], input[1*2+0], blend);
    tmp_in[2*2+0] = _mm_blendv_epi8(input[2*2+1], input[2*2+0], blend);
    tmp_in[3*2+0] = _mm_blendv_epi8(input[3*2+1], input[3*2+0], blend);
    tmp_in[0*2+1] = _mm_blendv_epi8(input[0*2+0], input[0*2+1], blend);
    tmp_in[1*2+1] = _mm_blendv_epi8(input[1*2+0], input[1*2+1], blend);
    tmp_in[2*2+1] = _mm_blendv_epi8(input[2*2+0], input[2*2+1], blend);
    tmp_in[3*2+1] = _mm_blendv_epi8(input[3*2+0], input[3*2+1], blend);
    key[0*2+0] = _mm_xor_si128(xor[0*2+0], roundkeys[28]);
    key[1*2+0] = _mm_xor_si128(xor[1*2+0], roundkeys[28]);
    key[2*2+0] = _mm_xor_si128(xor[2*2+0], roundkeys[28]);
    key[3*2+0] = _mm_xor_si128(xor[3*2+0], roundkeys[28]);
    key[0*2+1] = _mm_xor_si128(xor[0*2+1], roundkeys[29]);
    key[1*2+1] = _mm_xor_si128(xor[1*2+1], roundkeys[29]);
    key[2*2+1] = _mm_xor_si128(xor[2*2+1], roundkeys[29]);
    key[3*2+1] = _mm_xor_si128(xor[3*2+1], roundkeys[29]);
    input[0*2+0] = _mm_shuffle_epi8(tmp_in[0*2+0], shuffle);
    input[1*2+0] = _mm_shuffle_epi8(tmp_in[1*2+0], shuffle);
    input[2*2+0] = _mm_shuffle_epi8(tmp_in[2*2+0], shuffle);
    input[3*2+0] = _mm_shuffle_epi8(tmp_in[3*2+0], shuffle);
    input[0*2+1] = _mm_shuffle_epi8(tmp_in[0*2+1], shuffle);
    input[1*2+1] = _mm_shuffle_epi8(tmp_in[1*2+1], shuffle);
    input[2*2+1] = _mm_shuffle_epi8(tmp_in[2*2+1], shuffle);
    input[3*2+1] = _mm_shuffle_epi8(tmp_in[3*2+1], shuffle);
    out[0*2+0] = _mm_aesenclast_si128(input[0*2+0], key[0*2+0]);
    out[1*2+0] = _mm_aesenclast_si128(input[1*2+0], key[1*2+0]);
    out[2*2+0] = _mm_aesenclast_si128(input[2*2+0], key[2*2+0]);
    out[3*2+0] = _mm_aesenclast_si128(input[3*2+0], key[3*2+0]);
    out[0*2+1] = _mm_aesenclast_si128(input[0*2+1], key[0*2+1]);
    out[1*2+1] = _mm_aesenclast_si128(input[1*2+1], key[1*2+1]);
    out[2*2+1] = _mm_aesenclast_si128(input[2*2+1], key[2*2+1]);
    out[3*2+1] = _mm_aesenclast_si128(input[3*2+1], key[3*2+1]);
    return;
}

static inline void rijndael256blockXORx6(__m128i out[6*2],
                                       const __m128i roundkeys[30],
                                       const __m128i in[6*2],
                                       const __m128i xor[6*2])
{
    __m128i key[6*2];
    __m128i tmp_in[6*2];
    __m128i shuffle = shufmask.v;
    __m128i blend = blendmask.v;
    __m128i input[6*2];
    input[0*2+0] = _mm_xor_si128(in[0*2+0], roundkeys[0]);
    input[1*2+0] = _mm_xor_si128(in[1*2+0], roundkeys[0]);
    input[2*2+0] = _mm_xor_si128(in[2*2+0], roundkeys[0]);
    input[3*2+0] = _mm_xor_si128(in[3*2+0], roundkeys[0]);
    input[4*2+0] = _mm_xor_si128(in[4*2+0], roundkeys[0]);
    input[5*2+0] = _mm_xor_si128(in[5*2+0], roundkeys[0]);
    input[0*2+1] = _mm_xor_si128(in[0*2+1], roundkeys[1]);
    input[1*2+1] = _mm_xor_si128(in[1*2+1], roundkeys[1]);
    input[2*2+1] = _mm_xor_si128(in[2*2+1], roundkeys[1]);
    input[3*2+1] = _mm_xor_si128(in[3*2+1], roundkeys[1]);
    input[4*2+1] = _mm_xor_si128(in[4*2+1], roundkeys[1]);
    input[5*2+1] = _mm_xor_si128(in[5*2+1], roundkeys[1]);
#ifdef __GNUC__
#ifdef __clang__
#pragma unroll 14
#else
#pragma GCC unroll 14
#endif
#endif
    for (int i = 1; i < 14; i++) {
        tmp_in[0*2+0] = _mm_blendv_epi8(input[0*2+1], input[0*2+0], blend);
        tmp_in[1*2+0] = _mm_blendv_epi8(input[1*2+1], input[1*2+0], blend);
        tmp_in[2*2+0] = _mm_blendv_epi8(input[2*2+1], input[2*2+0], blend);
        tmp_in[3*2+0] = _mm_blendv_epi8(input[3*2+1], input[3*2+0], blend);
        tmp_in[4*2+0] = _mm_blendv_epi8(input[4*2+1], input[4*2+0], blend);
        tmp_in[5*2+0] = _mm_blendv_epi8(input[5*2+1], input[5*2+0], blend);
        tmp_in[0*2+1] = _mm_blendv_epi8(input[0*2+0], input[0*2+1], blend);
        tmp_in[1*2+1] = _mm_blendv_epi8(input[1*2+0], input[1*2+1], blend);
        tmp_in[2*2+1] = _mm_blendv_epi8(input[2*2+0], input[2*2+1], blend);
        tmp_in[3*2+1] = _mm_blendv_epi8(input[3*2+0], input[3*2+1], blend);
        tmp_in[4*2+1] = _mm_blendv_epi8(input[4*2+0], input[4*2+1], blend);
        tmp_in[5*2+1] = _mm_blendv_epi8(input[5*2+0], input[5*2+1], blend);
        key[0] = roundkeys[2 * i];
        key[1] = roundkeys[2 * i + 1];
        input[0*2+0] = _mm_shuffle_epi8(tmp_in[0*2+0], shuffle);
        input[1*2+0] = _mm_shuffle_epi8(tmp_in[1*2+0], shuffle);
        input[2*2+0] = _mm_shuffle_epi8(tmp_in[2*2+0], shuffle);
        input[3*2+0] = _mm_shuffle_epi8(tmp_in[3*2+0], shuffle);
        input[4*2+0] = _mm_shuffle_epi8(tmp_in[4*2+0], shuffle);
        input[5*2+0] = _mm_shuffle_epi8(tmp_in[5*2+0], shuffle);
        input[0*2+1] = _mm_shuffle_epi8(tmp_in[0*2+1], shuffle);
        input[1*2+1] = _mm_shuffle_epi8(tmp_in[1*2+1], shuffle);
        input[2*2+1] = _mm_shuffle_epi8(tmp_in[2*2+1], shuffle);
        input[3*2+1] = _mm_shuffle_epi8(tmp_in[3*2+1], shuffle);
        input[4*2+1] = _mm_shuffle_epi8(tmp_in[4*2+1], shuffle);
        input[5*2+1] = _mm_shuffle_epi8(tmp_in[5*2+1], shuffle);
        input[0*2+0] = _mm_aesenc_si128(input[0*2+0], key[0]);
        input[1*2+0] = _mm_aesenc_si128(input[1*2+0], key[0]);
        input[2*2+0] = _mm_aesenc_si128(input[2*2+0], key[0]);
        input[3*2+0] = _mm_aesenc_si128(input[3*2+0], key[0]);
        input[4*2+0] = _mm_aesenc_si128(input[4*2+0], key[0]);
        input[5*2+0] = _mm_aesenc_si128(input[5*2+0], key[0]);
        input[0*2+1] = _mm_aesenc_si128(input[0*2+1], key[1]);
        input[1*2+1] = _mm_aesenc_si128(input[1*2+1], key[1]);
        input[2*2+1] = _mm_aesenc_si128(input[2*2+1], key[1]);
        input[3*2+1] = _mm_aesenc_si128(input[3*2+1], key[1]);
        input[4*2+1] = _mm_aesenc_si128(input[4*2+1], key[1]);
        input[5*2+1] = _mm_aesenc_si128(input[5*2+1], key[1]);
    }
    tmp_in[0*2+0] = _mm_blendv_epi8(input[0*2+1], input[0*2+0], blend);
    tmp_in[1*2+0] = _mm_blendv_epi8(input[1*2+1], input[1*2+0], blend);
    tmp_in[2*2+0] = _mm_blendv_epi8(input[2*2+1], input[2*2+0], blend);
    tmp_in[3*2+0] = _mm_blendv_epi8(input[3*2+1], input[3*2+0], blend);
    tmp_in[4*2+0] = _mm_blendv_epi8(input[4*2+1], input[4*2+0], blend);
    tmp_in[5*2+0] = _mm_blendv_epi8(input[5*2+1], input[5*2+0], blend);
    tmp_in[0*2+1] = _mm_blendv_epi8(input[0*2+0], input[0*2+1], blend);
    tmp_in[1*2+1] = _mm_blendv_epi8(input[1*2+0], input[1*2+1], blend);
    tmp_in[2*2+1] = _mm_blendv_epi8(input[2*2+0], input[2*2+1], blend);
    tmp_in[3*2+1] = _mm_blendv_epi8(input[3*2+0], input[3*2+1], blend);
    tmp_in[4*2+1] = _mm_blendv_epi8(input[4*2+0], input[4*2+1], blend);
    tmp_in[5*2+1] = _mm_blendv_epi8(input[5*2+0], input[5*2+1], blend);
    key[0*2+0] = _mm_xor_si128(xor[0*2+0], roundkeys[28]);
    key[1*2+0] = _mm_xor_si128(xor[1*2+0], roundkeys[28]);
    key[2*2+0] = _mm_xor_si128(xor[2*2+0], roundkeys[28]);
    key[3*2+0] = _mm_xor_si128(xor[3*2+0], roundkeys[28]);
    key[4*2+0] = _mm_xor_si128(xor[4*2+0], roundkeys[28]);
    key[5*2+0] = _mm_xor_si128(xor[5*2+0], roundkeys[28]);
    key[0*2+1] = _mm_xor_si128(xor[0*2+1], roundkeys[29]);
    key[1*2+1] = _mm_xor_si128(xor[1*2+1], roundkeys[29]);
    key[2*2+1] = _mm_xor_si128(xor[2*2+1], roundkeys[29]);
    key[3*2+1] = _mm_xor_si128(xor[3*2+1], roundkeys[29]);
    key[4*2+1] = _mm_xor_si128(xor[4*2+1], roundkeys[29]);
    key[5*2+1] = _mm_xor_si128(xor[5*2+1], roundkeys[29]);
    input[0*2+0] = _mm_shuffle_epi8(tmp_in[0*2+0], shuffle);
    input[1*2+0] = _mm_shuffle_epi8(tmp_in[1*2+0], shuffle);
    input[2*2+0] = _mm_shuffle_epi8(tmp_in[2*2+0], shuffle);
    input[3*2+0] = _mm_shuffle_epi8(tmp_in[3*2+0], shuffle);
    input[4*2+0] = _mm_shuffle_epi8(tmp_in[4*2+0], shuffle);
    input[5*2+0] = _mm_shuffle_epi8(tmp_in[5*2+0], shuffle);
    input[0*2+1] = _mm_shuffle_epi8(tmp_in[0*2+1], shuffle);
    input[1*2+1] = _mm_shuffle_epi8(tmp_in[1*2+1], shuffle);
    input[2*2+1] = _mm_shuffle_epi8(tmp_in[2*2+1], shuffle);
    input[3*2+1] = _mm_shuffle_epi8(tmp_in[3*2+1], shuffle);
    input[4*2+1] = _mm_shuffle_epi8(tmp_in[4*2+1], shuffle);
    input[5*2+1] = _mm_shuffle_epi8(tmp_in[5*2+1], shuffle);
    out[0*2+0] = _mm_aesenclast_si128(input[0*2+0], key[0*2+0]);
    out[1*2+0] = _mm_aesenclast_si128(input[1*2+0], key[1*2+0]);
    out[2*2+0] = _mm_aesenclast_si128(input[2*2+0], key[2*2+0]);
    out[3*2+0] = _mm_aesenclast_si128(input[3*2+0], key[3*2+0]);
    out[4*2+0] = _mm_aesenclast_si128(input[4*2+0], key[4*2+0]);
    out[5*2+0] = _mm_aesenclast_si128(input[5*2+0], key[5*2+0]);
    out[0*2+1] = _mm_aesenclast_si128(input[0*2+1], key[0*2+1]);
    out[1*2+1] = _mm_aesenclast_si128(input[1*2+1], key[1*2+1]);
    out[2*2+1] = _mm_aesenclast_si128(input[2*2+1], key[2*2+1]);
    out[3*2+1] = _mm_aesenclast_si128(input[3*2+1], key[3*2+1]);
    out[4*2+1] = _mm_aesenclast_si128(input[4*2+1], key[4*2+1]);
    out[5*2+1] = _mm_aesenclast_si128(input[5*2+1], key[5*2+1]);
    return;
}

static inline void rijndael256blockXORx8(__m128i out[8*2],
                                       const __m128i roundkeys[30],
                                       const __m128i in[8*2],
                                       const __m128i xor[8*2])
{
    __m128i key[8*2];
    __m128i tmp_in[8*2];
    __m128i shuffle = shufmask.v;
    __m128i blend = blendmask.v;
    __m128i input[8*2];
    input[0*2+0] = _mm_xor_si128(in[0*2+0], roundkeys[0]);
    input[1*2+0] = _mm_xor_si128(in[1*2+0], roundkeys[0]);
    input[2*2+0] = _mm_xor_si128(in[2*2+0], roundkeys[0]);
    input[3*2+0] = _mm_xor_si128(in[3*2+0], roundkeys[0]);
    input[4*2+0] = _mm_xor_si128(in[4*2+0], roundkeys[0]);
    input[5*2+0] = _mm_xor_si128(in[5*2+0], roundkeys[0]);
    input[6*2+0] = _mm_xor_si128(in[6*2+0], roundkeys[0]);
    input[7*2+0] = _mm_xor_si128(in[7*2+0], roundkeys[0]);
    input[0*2+1] = _mm_xor_si128(in[0*2+1], roundkeys[1]);
    input[1*2+1] = _mm_xor_si128(in[1*2+1], roundkeys[1]);
    input[2*2+1] = _mm_xor_si128(in[2*2+1], roundkeys[1]);
    input[3*2+1] = _mm_xor_si128(in[3*2+1], roundkeys[1]);
    input[4*2+1] = _mm_xor_si128(in[4*2+1], roundkeys[1]);
    input[5*2+1] = _mm_xor_si128(in[5*2+1], roundkeys[1]);
    input[6*2+1] = _mm_xor_si128(in[6*2+1], roundkeys[1]);
    input[7*2+1] = _mm_xor_si128(in[7*2+1], roundkeys[1]);
#ifdef __GNUC__
#ifdef __clang__
#pragma unroll 14
#else
#pragma GCC unroll 14
#endif
#endif
    for (int i = 1; i < 14; i++) {
        tmp_in[0*2+0] = _mm_blendv_epi8(input[0*2+1], input[0*2+0], blend);
        tmp_in[1*2+0] = _mm_blendv_epi8(input[1*2+1], input[1*2+0], blend);
        tmp_in[2*2+0] = _mm_blendv_epi8(input[2*2+1], input[2*2+0], blend);
        tmp_in[3*2+0] = _mm_blendv_epi8(input[3*2+1], input[3*2+0], blend);
        tmp_in[4*2+0] = _mm_blendv_epi8(input[4*2+1], input[4*2+0], blend);
        tmp_in[5*2+0] = _mm_blendv_epi8(input[5*2+1], input[5*2+0], blend);
        tmp_in[6*2+0] = _mm_blendv_epi8(input[6*2+1], input[6*2+0], blend);
        tmp_in[7*2+0] = _mm_blendv_epi8(input[7*2+1], input[7*2+0], blend);
        tmp_in[0*2+1] = _mm_blendv_epi8(input[0*2+0], input[0*2+1], blend);
        tmp_in[1*2+1] = _mm_blendv_epi8(input[1*2+0], input[1*2+1], blend);
        tmp_in[2*2+1] = _mm_blendv_epi8(input[2*2+0], input[2*2+1], blend);
        tmp_in[3*2+1] = _mm_blendv_epi8(input[3*2+0], input[3*2+1], blend);
        tmp_in[4*2+1] = _mm_blendv_epi8(input[4*2+0], input[4*2+1], blend);
        tmp_in[5*2+1] = _mm_blendv_epi8(input[5*2+0], input[5*2+1], blend);
        tmp_in[6*2+1] = _mm_blendv_epi8(input[6*2+0], input[6*2+1], blend);
        tmp_in[7*2+1] = _mm_blendv_epi8(input[7*2+0], input[7*2+1], blend);
        key[0] = roundkeys[2 * i];
        key[1] = roundkeys[2 * i + 1];
        input[0*2+0] = _mm_shuffle_epi8(tmp_in[0*2+0], shuffle);
        input[1*2+0] = _mm_shuffle_epi8(tmp_in[1*2+0], shuffle);
        input[2*2+0] = _mm_shuffle_epi8(tmp_in[2*2+0], shuffle);
        input[3*2+0] = _mm_shuffle_epi8(tmp_in[3*2+0], shuffle);
        input[4*2+0] = _mm_shuffle_epi8(tmp_in[4*2+0], shuffle);
        input[5*2+0] = _mm_shuffle_epi8(tmp_in[5*2+0], shuffle);
        input[6*2+0] = _mm_shuffle_epi8(tmp_in[6*2+0], shuffle);
        input[7*2+0] = _mm_shuffle_epi8(tmp_in[7*2+0], shuffle);
        input[0*2+1] = _mm_shuffle_epi8(tmp_in[0*2+1], shuffle);
        input[1*2+1] = _mm_shuffle_epi8(tmp_in[1*2+1], shuffle);
        input[2*2+1] = _mm_shuffle_epi8(tmp_in[2*2+1], shuffle);
        input[3*2+1] = _mm_shuffle_epi8(tmp_in[3*2+1], shuffle);
        input[4*2+1] = _mm_shuffle_epi8(tmp_in[4*2+1], shuffle);
        input[5*2+1] = _mm_shuffle_epi8(tmp_in[5*2+1], shuffle);
        input[6*2+1] = _mm_shuffle_epi8(tmp_in[6*2+1], shuffle);
        input[7*2+1] = _mm_shuffle_epi8(tmp_in[7*2+1], shuffle);
        input[0*2+0] = _mm_aesenc_si128(input[0*2+0], key[0]);
        input[1*2+0] = _mm_aesenc_si128(input[1*2+0], key[0]);
        input[2*2+0] = _mm_aesenc_si128(input[2*2+0], key[0]);
        input[3*2+0] = _mm_aesenc_si128(input[3*2+0], key[0]);
        input[4*2+0] = _mm_aesenc_si128(input[4*2+0], key[0]);
        input[5*2+0] = _mm_aesenc_si128(input[5*2+0], key[0]);
        input[6*2+0] = _mm_aesenc_si128(input[6*2+0], key[0]);
        input[7*2+0] = _mm_aesenc_si128(input[7*2+0], key[0]);
        input[0*2+1] = _mm_aesenc_si128(input[0*2+1], key[1]);
        input[1*2+1] = _mm_aesenc_si128(input[1*2+1], key[1]);
        input[2*2+1] = _mm_aesenc_si128(input[2*2+1], key[1]);
        input[3*2+1] = _mm_aesenc_si128(input[3*2+1], key[1]);
        input[4*2+1] = _mm_aesenc_si128(input[4*2+1], key[1]);
        input[5*2+1] = _mm_aesenc_si128(input[5*2+1], key[1]);
        input[6*2+1] = _mm_aesenc_si128(input[6*2+1], key[1]);
        input[7*2+1] = _mm_aesenc_si128(input[7*2+1], key[1]);
    }
    tmp_in[0*2+0] = _mm_blendv_epi8(input[0*2+1], input[0*2+0], blend);
    tmp_in[1*2+0] = _mm_blendv_epi8(input[1*2+1], input[1*2+0], blend);
    tmp_in[2*2+0] = _mm_blendv_epi8(input[2*2+1], input[2*2+0], blend);
    tmp_in[3*2+0] = _mm_blendv_epi8(input[3*2+1], input[3*2+0], blend);
    tmp_in[4*2+0] = _mm_blendv_epi8(input[4*2+1], input[4*2+0], blend);
    tmp_in[5*2+0] = _mm_blendv_epi8(input[5*2+1], input[5*2+0], blend);
    tmp_in[6*2+0] = _mm_blendv_epi8(input[6*2+1], input[6*2+0], blend);
    tmp_in[7*2+0] = _mm_blendv_epi8(input[7*2+1], input[7*2+0], blend);
    tmp_in[0*2+1] = _mm_blendv_epi8(input[0*2+0], input[0*2+1], blend);
    tmp_in[1*2+1] = _mm_blendv_epi8(input[1*2+0], input[1*2+1], blend);
    tmp_in[2*2+1] = _mm_blendv_epi8(input[2*2+0], input[2*2+1], blend);
    tmp_in[3*2+1] = _mm_blendv_epi8(input[3*2+0], input[3*2+1], blend);
    tmp_in[4*2+1] = _mm_blendv_epi8(input[4*2+0], input[4*2+1], blend);
    tmp_in[5*2+1] = _mm_blendv_epi8(input[5*2+0], input[5*2+1], blend);
    tmp_in[6*2+1] = _mm_blendv_epi8(input[6*2+0], input[6*2+1], blend);
    tmp_in[7*2+1] = _mm_blendv_epi8(input[7*2+0], input[7*2+1], blend);
    key[0*2+0] = _mm_xor_si128(xor[0*2+0], roundkeys[28]);
    key[1*2+0] = _mm_xor_si128(xor[1*2+0], roundkeys[28]);
    key[2*2+0] = _mm_xor_si128(xor[2*2+0], roundkeys[28]);
    key[3*2+0] = _mm_xor_si128(xor[3*2+0], roundkeys[28]);
    key[4*2+0] = _mm_xor_si128(xor[4*2+0], roundkeys[28]);
    key[5*2+0] = _mm_xor_si128(xor[5*2+0], roundkeys[28]);
    key[6*2+0] = _mm_xor_si128(xor[6*2+0], roundkeys[28]);
    key[7*2+0] = _mm_xor_si128(xor[7*2+0], roundkeys[28]);
    key[0*2+1] = _mm_xor_si128(xor[0*2+1], roundkeys[29]);
    key[1*2+1] = _mm_xor_si128(xor[1*2+1], roundkeys[29]);
    key[2*2+1] = _mm_xor_si128(xor[2*2+1], roundkeys[29]);
    key[3*2+1] = _mm_xor_si128(xor[3*2+1], roundkeys[29]);
    key[4*2+1] = _mm_xor_si128(xor[4*2+1], roundkeys[29]);
    key[5*2+1] = _mm_xor_si128(xor[5*2+1], roundkeys[29]);
    key[6*2+1] = _mm_xor_si128(xor[6*2+1], roundkeys[29]);
    key[7*2+1] = _mm_xor_si128(xor[7*2+1], roundkeys[29]);
    input[0*2+0] = _mm_shuffle_epi8(tmp_in[0*2+0], shuffle);
    input[1*2+0] = _mm_shuffle_epi8(tmp_in[1*2+0], shuffle);
    input[2*2+0] = _mm_shuffle_epi8(tmp_in[2*2+0], shuffle);
    input[3*2+0] = _mm_shuffle_epi8(tmp_in[3*2+0], shuffle);
    input[4*2+0] = _mm_shuffle_epi8(tmp_in[4*2+0], shuffle);
    input[5*2+0] = _mm_shuffle_epi8(tmp_in[5*2+0], shuffle);
    input[6*2+0] = _mm_shuffle_epi8(tmp_in[6*2+0], shuffle);
    input[7*2+0] = _mm_shuffle_epi8(tmp_in[7*2+0], shuffle);
    input[0*2+1] = _mm_shuffle_epi8(tmp_in[0*2+1], shuffle);
    input[1*2+1] = _mm_shuffle_epi8(tmp_in[1*2+1], shuffle);
    input[2*2+1] = _mm_shuffle_epi8(tmp_in[2*2+1], shuffle);
    input[3*2+1] = _mm_shuffle_epi8(tmp_in[3*2+1], shuffle);
    input[4*2+1] = _mm_shuffle_epi8(tmp_in[4*2+1], shuffle);
    input[5*2+1] = _mm_shuffle_epi8(tmp_in[5*2+1], shuffle);
    input[6*2+1] = _mm_shuffle_epi8(tmp_in[6*2+1], shuffle);
    input[7*2+1] = _mm_shuffle_epi8(tmp_in[7*2+1], shuffle);
    out[0*2+0] = _mm_aesenclast_si128(input[0*2+0], key[0*2+0]);
    out[1*2+0] = _mm_aesenclast_si128(input[1*2+0], key[1*2+0]);
    out[2*2+0] = _mm_aesenclast_si128(input[2*2+0], key[2*2+0]);
    out[3*2+0] = _mm_aesenclast_si128(input[3*2+0], key[3*2+0]);
    out[4*2+0] = _mm_aesenclast_si128(input[4*2+0], key[4*2+0]);
    out[5*2+0] = _mm_aesenclast_si128(input[5*2+0], key[5*2+0]);
    out[6*2+0] = _mm_aesenclast_si128(input[6*2+0], key[6*2+0]);
    out[7*2+0] = _mm_aesenclast_si128(input[7*2+0], key[7*2+0]);
    out[0*2+1] = _mm_aesenclast_si128(input[0*2+1], key[0*2+1]);
    out[1*2+1] = _mm_aesenclast_si128(input[1*2+1], key[1*2+1]);
    out[2*2+1] = _mm_aesenclast_si128(input[2*2+1], key[2*2+1]);
    out[3*2+1] = _mm_aesenclast_si128(input[3*2+1], key[3*2+1]);
    out[4*2+1] = _mm_aesenclast_si128(input[4*2+1], key[4*2+1]);
    out[5*2+1] = _mm_aesenclast_si128(input[5*2+1], key[5*2+1]);
    out[6*2+1] = _mm_aesenclast_si128(input[6*2+1], key[6*2+1]);
    out[7*2+1] = _mm_aesenclast_si128(input[7*2+1], key[7*2+1]);
    return;
}

void rijndael256_key_expansion(__m128i outKey[30], const __m128i inKey[2])
{
#define KEYGENASSIST(i, j)                                                     \
    xmm2 = _mm_aeskeygenassist_si128(xmm3, j);                                 \
    xmm2 = _mm_shuffle_epi32(xmm2, 0xff);                                      \
    xmm4 = _mm_slli_si128(xmm1, 0x4);                                          \
    xmm1 = _mm_xor_si128(xmm1, xmm4);                                          \
    xmm4 = _mm_slli_si128(xmm4, 0x4);                                          \
    xmm1 = _mm_xor_si128(xmm1, xmm4);                                          \
    xmm4 = _mm_slli_si128(xmm4, 0x4);                                          \
    xmm1 = _mm_xor_si128(xmm1, xmm4);                                          \
    xmm1 = _mm_xor_si128(xmm1, xmm2);                                          \
    outKey[2 * (i)] = xmm1;                                                    \
    xmm4 = _mm_aeskeygenassist_si128(xmm1, 0x00);                              \
    xmm2 = _mm_shuffle_epi32(xmm4, 0xaa);                                      \
    xmm4 = _mm_slli_si128(xmm3, 0x4);                                          \
    xmm3 = _mm_xor_si128(xmm3, xmm4);                                          \
    xmm4 = _mm_slli_si128(xmm4, 0x4);                                          \
    xmm3 = _mm_xor_si128(xmm3, xmm4);                                          \
    xmm4 = _mm_slli_si128(xmm4, 0x4);                                          \
    xmm3 = _mm_xor_si128(xmm3, xmm4);                                          \
    xmm3 = _mm_xor_si128(xmm3, xmm2);                                          \
    outKey[2 * (i) + 1] = xmm3;

    __m128i xmm1, xmm2, xmm3, xmm4;
    xmm1 = inKey[0];
    xmm3 = inKey[1];
    outKey[0] = xmm1;
    outKey[1] = xmm3;
    KEYGENASSIST(1, 0x01)
    KEYGENASSIST(2, 0x02)
    KEYGENASSIST(3, 0x04)
    KEYGENASSIST(4, 0x08)
    KEYGENASSIST(5, 0x10)
    KEYGENASSIST(6, 0x20)
    KEYGENASSIST(7, 0x40)
    KEYGENASSIST(8, 0x80)
    KEYGENASSIST(9, 0x1b)
    KEYGENASSIST(10, 0x36)
    KEYGENASSIST(11, 0x6c)
    KEYGENASSIST(12, 0xd8)
    KEYGENASSIST(13, 0xab)
    KEYGENASSIST(14, 0x4d)
#undef KEYGENASSIST
}
