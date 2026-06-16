#pragma once

#include <immintrin.h>
#include <inttypes.h>
#include <stddef.h>
#include <string.h>
#define BUFFSIZE 16
#define DOUBLE_WORDSIZE 128
#define LIMBMASK ((((uint64_t)1) << LIMBBITS) - 1)
#define LIMBMASK2 ((((uint64_t)1) << LIMBBITS2) - 1)
typedef uint64_t baseint_t;
typedef struct field_elem_single {
    __m128i val[1];
} field_elem_t;
typedef struct field_elem_double {
    __m128i val[2];
} dfield_elem_t;

#ifdef DEBUG
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#define PRINT_VECTOR(...) print_vector(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#define PRINT_VECTOR(...)
#endif

#define PRINT_FIELD_ELEM(elem)                                                 \
    DEBUG_PRINTF("%.16llx %.16llx\n", _mm_extract_epi64(elem.val[0], 1),       \
                 _mm_extract_epi64(elem.val[0], 0));

#define PRINT_DFIELD_ELEM(elem)                                                \
    DEBUG_PRINTF(                                                              \
        "%.16llx %.16llx %.16llx %.16llx\n",                                   \
        _mm_extract_epi64(elem.val[0], 1), _mm_extract_epi64(elem.val[0], 0),  \
        _mm_extract_epi64(elem.val[1], 1), _mm_extract_epi64(elem.val[1], 0));

#define HERE DEBUG_PRINTF("%s:%d\n", __FILE__, __LINE__);
static inline __attribute__((always_inline)) int carry_round(field_elem_t *res,
                                                             dfield_elem_t *a)
{
    __m128i tt[1] = {0};
    dfield_elem_t t;
    res->val[0] = _mm_xor_si128(a->val[0], a->val[1]);
    t.val[0] = _mm_shuffle_epi32(a->val[1], 0x4E);
    //* x^7
    tt[0] = _mm_xor_si128(tt[0], _mm_srli_epi64(t.val[0], 57));
    //* x^2
    tt[0] = _mm_xor_si128(tt[0], _mm_srli_epi64(t.val[0], 62));
    //* x^1
    tt[0] = _mm_xor_si128(tt[0], _mm_srli_epi64(t.val[0], 63));
    res->val[0] = _mm_xor_si128(res->val[0], tt[0]);
    a->val[1] = _mm_xor_si128(
        a->val[1], _mm_and_si128(_mm_set_epi64x(0, 0xFFFFFFFFFFFFFFFF), tt[0]));
    //* x^7
    res->val[0] = _mm_xor_si128(res->val[0], _mm_slli_epi64(a->val[1], 7));
    //* x^2
    res->val[0] = _mm_xor_si128(res->val[0], _mm_slli_epi64(a->val[1], 2));
    //* x^1
    res->val[0] = _mm_xor_si128(res->val[0], _mm_slli_epi64(a->val[1], 1));
    return 0;
}


static inline __attribute__((always_inline)) int carry_round8(field_elem_t res[8],
                                                             dfield_elem_t a[8])
{
    __m128i tt[1][8] = {0};
    dfield_elem_t t[8];
    res[0].val[0] = _mm_xor_si128(a[0].val[0], a[0].val[1]);
    res[1].val[0] = _mm_xor_si128(a[1].val[0], a[1].val[1]);
    res[2].val[0] = _mm_xor_si128(a[2].val[0], a[2].val[1]);
    res[3].val[0] = _mm_xor_si128(a[3].val[0], a[3].val[1]);
    res[4].val[0] = _mm_xor_si128(a[4].val[0], a[4].val[1]);
    res[5].val[0] = _mm_xor_si128(a[5].val[0], a[5].val[1]);
    res[6].val[0] = _mm_xor_si128(a[6].val[0], a[6].val[1]);
    res[7].val[0] = _mm_xor_si128(a[7].val[0], a[7].val[1]);
    t[0].val[0] = _mm_shuffle_epi32(a[0].val[1], 0x4E);
    t[1].val[0] = _mm_shuffle_epi32(a[1].val[1], 0x4E);
    t[2].val[0] = _mm_shuffle_epi32(a[2].val[1], 0x4E);
    t[3].val[0] = _mm_shuffle_epi32(a[3].val[1], 0x4E);
    t[4].val[0] = _mm_shuffle_epi32(a[4].val[1], 0x4E);
    t[5].val[0] = _mm_shuffle_epi32(a[5].val[1], 0x4E);
    t[6].val[0] = _mm_shuffle_epi32(a[6].val[1], 0x4E);
    t[7].val[0] = _mm_shuffle_epi32(a[7].val[1], 0x4E);
    //* x^7
    tt[0][0] = _mm_xor_si128(tt[0][0], _mm_srli_epi64(t[0].val[0], 57));
    tt[0][1] = _mm_xor_si128(tt[0][1], _mm_srli_epi64(t[1].val[0], 57));
    tt[0][2] = _mm_xor_si128(tt[0][2], _mm_srli_epi64(t[2].val[0], 57));
    tt[0][3] = _mm_xor_si128(tt[0][3], _mm_srli_epi64(t[3].val[0], 57));
    tt[0][4] = _mm_xor_si128(tt[0][4], _mm_srli_epi64(t[4].val[0], 57));
    tt[0][5] = _mm_xor_si128(tt[0][5], _mm_srli_epi64(t[5].val[0], 57));
    tt[0][6] = _mm_xor_si128(tt[0][6], _mm_srli_epi64(t[6].val[0], 57));
    tt[0][7] = _mm_xor_si128(tt[0][7], _mm_srli_epi64(t[7].val[0], 57));
    //* x^2
    tt[0][0] = _mm_xor_si128(tt[0][0], _mm_srli_epi64(t[0].val[0], 62));
    tt[0][1] = _mm_xor_si128(tt[0][1], _mm_srli_epi64(t[1].val[0], 62));
    tt[0][2] = _mm_xor_si128(tt[0][2], _mm_srli_epi64(t[2].val[0], 62));
    tt[0][3] = _mm_xor_si128(tt[0][3], _mm_srli_epi64(t[3].val[0], 62));
    tt[0][4] = _mm_xor_si128(tt[0][4], _mm_srli_epi64(t[4].val[0], 62));
    tt[0][5] = _mm_xor_si128(tt[0][5], _mm_srli_epi64(t[5].val[0], 62));
    tt[0][6] = _mm_xor_si128(tt[0][6], _mm_srli_epi64(t[6].val[0], 62));
    tt[0][7] = _mm_xor_si128(tt[0][7], _mm_srli_epi64(t[7].val[0], 62));
    //* x^1
    tt[0][0] = _mm_xor_si128(tt[0][0], _mm_srli_epi64(t[0].val[0], 63));
    tt[0][1] = _mm_xor_si128(tt[0][1], _mm_srli_epi64(t[1].val[0], 63));
    tt[0][2] = _mm_xor_si128(tt[0][2], _mm_srli_epi64(t[2].val[0], 63));
    tt[0][3] = _mm_xor_si128(tt[0][3], _mm_srli_epi64(t[3].val[0], 63));
    tt[0][4] = _mm_xor_si128(tt[0][4], _mm_srli_epi64(t[4].val[0], 63));
    tt[0][5] = _mm_xor_si128(tt[0][5], _mm_srli_epi64(t[5].val[0], 63));
    tt[0][6] = _mm_xor_si128(tt[0][6], _mm_srli_epi64(t[6].val[0], 63));
    tt[0][7] = _mm_xor_si128(tt[0][7], _mm_srli_epi64(t[7].val[0], 63));
    res[0].val[0] = _mm_xor_si128(res[0].val[0], tt[0][0]);
    res[1].val[0] = _mm_xor_si128(res[1].val[0], tt[0][1]);
    res[2].val[0] = _mm_xor_si128(res[2].val[0], tt[0][2]);
    res[3].val[0] = _mm_xor_si128(res[3].val[0], tt[0][3]);
    res[4].val[0] = _mm_xor_si128(res[4].val[0], tt[0][4]);
    res[5].val[0] = _mm_xor_si128(res[5].val[0], tt[0][5]);
    res[6].val[0] = _mm_xor_si128(res[6].val[0], tt[0][6]);
    res[7].val[0] = _mm_xor_si128(res[7].val[0], tt[0][7]);
    a[0].val[1] = _mm_xor_si128(a[0].val[1], _mm_and_si128(_mm_set_epi64x(0, 0xFFFFFFFFFFFFFFFF), tt[0][0]));
    a[1].val[1] = _mm_xor_si128(a[1].val[1], _mm_and_si128(_mm_set_epi64x(0, 0xFFFFFFFFFFFFFFFF), tt[0][1]));
    a[2].val[1] = _mm_xor_si128(a[2].val[1], _mm_and_si128(_mm_set_epi64x(0, 0xFFFFFFFFFFFFFFFF), tt[0][2]));
    a[3].val[1] = _mm_xor_si128(a[3].val[1], _mm_and_si128(_mm_set_epi64x(0, 0xFFFFFFFFFFFFFFFF), tt[0][3]));
    a[4].val[1] = _mm_xor_si128(a[4].val[1], _mm_and_si128(_mm_set_epi64x(0, 0xFFFFFFFFFFFFFFFF), tt[0][4]));
    a[5].val[1] = _mm_xor_si128(a[5].val[1], _mm_and_si128(_mm_set_epi64x(0, 0xFFFFFFFFFFFFFFFF), tt[0][5]));
    a[6].val[1] = _mm_xor_si128(a[6].val[1], _mm_and_si128(_mm_set_epi64x(0, 0xFFFFFFFFFFFFFFFF), tt[0][6]));
    a[7].val[1] = _mm_xor_si128(a[7].val[1], _mm_and_si128(_mm_set_epi64x(0, 0xFFFFFFFFFFFFFFFF), tt[0][7]));
    //* x^7
    res[0].val[0] = _mm_xor_si128(res[0].val[0], _mm_slli_epi64(a[0].val[1], 7));
    res[1].val[0] = _mm_xor_si128(res[1].val[0], _mm_slli_epi64(a[1].val[1], 7));
    res[2].val[0] = _mm_xor_si128(res[2].val[0], _mm_slli_epi64(a[2].val[1], 7));
    res[3].val[0] = _mm_xor_si128(res[3].val[0], _mm_slli_epi64(a[3].val[1], 7));
    res[4].val[0] = _mm_xor_si128(res[4].val[0], _mm_slli_epi64(a[4].val[1], 7));
    res[5].val[0] = _mm_xor_si128(res[5].val[0], _mm_slli_epi64(a[5].val[1], 7));
    res[6].val[0] = _mm_xor_si128(res[6].val[0], _mm_slli_epi64(a[6].val[1], 7));
    res[7].val[0] = _mm_xor_si128(res[7].val[0], _mm_slli_epi64(a[7].val[1], 7));
    //* x^2
    res[0].val[0] = _mm_xor_si128(res[0].val[0], _mm_slli_epi64(a[0].val[1], 2));
    res[1].val[0] = _mm_xor_si128(res[1].val[0], _mm_slli_epi64(a[1].val[1], 2));
    res[2].val[0] = _mm_xor_si128(res[2].val[0], _mm_slli_epi64(a[2].val[1], 2));
    res[3].val[0] = _mm_xor_si128(res[3].val[0], _mm_slli_epi64(a[3].val[1], 2));
    res[4].val[0] = _mm_xor_si128(res[4].val[0], _mm_slli_epi64(a[4].val[1], 2));
    res[5].val[0] = _mm_xor_si128(res[5].val[0], _mm_slli_epi64(a[5].val[1], 2));
    res[6].val[0] = _mm_xor_si128(res[6].val[0], _mm_slli_epi64(a[6].val[1], 2));
    res[7].val[0] = _mm_xor_si128(res[7].val[0], _mm_slli_epi64(a[7].val[1], 2));
    //* x^1
    res[0].val[0] = _mm_xor_si128(res[0].val[0], _mm_slli_epi64(a[0].val[1], 1));
    res[1].val[0] = _mm_xor_si128(res[1].val[0], _mm_slli_epi64(a[1].val[1], 1));
    res[2].val[0] = _mm_xor_si128(res[2].val[0], _mm_slli_epi64(a[2].val[1], 1));
    res[3].val[0] = _mm_xor_si128(res[3].val[0], _mm_slli_epi64(a[3].val[1], 1));
    res[4].val[0] = _mm_xor_si128(res[4].val[0], _mm_slli_epi64(a[4].val[1], 1));
    res[5].val[0] = _mm_xor_si128(res[5].val[0], _mm_slli_epi64(a[5].val[1], 1));
    res[6].val[0] = _mm_xor_si128(res[6].val[0], _mm_slli_epi64(a[6].val[1], 1));
    res[7].val[0] = _mm_xor_si128(res[7].val[0], _mm_slli_epi64(a[7].val[1], 1));
    return 0;
}

typedef union {
    __m128i v;
    uint64_t i[2];
    uint32_t i32[4];
    uint8_t  i8[16];
} pvector;

// static inline __attribute__((always_inline)) int carry_round(field_elem_t *res,
//                                                              dfield_elem_t *a)
// {
//     __m128i tmp[3];
//     __m128i t;

//     const pvector poly = {.i = {0x87, 0}}; // x^10 + x^5 + x^2+ 1
//     tmp[1] = _mm_clmulepi64_si128(a->val[1], poly.v, 1);  
//     tmp[0] = _mm_clmulepi64_si128(a->val[1], poly.v, 0);  
//     tmp[2] = _mm_clmulepi64_si128(tmp[1], poly.v, 1); 
//     t = (__m128i) _mm_shuffle_ps((__m128)tmp[2], (__m128)tmp[1], 0x44);
//     res->val[0] = _mm_xor_si128(a->val[0], tmp[0]);
//     res->val[0] = _mm_xor_si128(res->val[0], t);
//     return 0;
// }



static inline __attribute__((always_inline)) int _carry_round(field_elem_t *res,
                                                              field_elem_t *a)
{
    if (res != a) {
        memcpy(res, a, sizeof(field_elem_t));
    }
    return 0;
}

static inline __attribute__((always_inline)) int reduce(field_elem_t *res,
                                                        const field_elem_t *a)
{
    if (res != a) {
        memcpy(res, a, sizeof(field_elem_t));
    }
    return 0;
}

static inline __attribute__((always_inline)) int
field_mul(field_elem_t *res, const field_elem_t *a, const field_elem_t *b)
{
    dfield_elem_t aa;
    __m128i acc = {0};
    __m128i d[3] = {0};
    // clmul(a[0], b[0])
    acc = _mm_clmulepi64_si128(a->val[0], b->val[0], 0);
    d[0] = _mm_xor_si128(d[0], acc);
    // clmul(a[0], b[1])
    acc = _mm_clmulepi64_si128(a->val[0], b->val[0], 16);
    d[1] = _mm_xor_si128(d[1], acc);
    // clmul(a[1], b[0])
    acc = _mm_clmulepi64_si128(a->val[0], b->val[0], 1);
    d[1] = _mm_xor_si128(d[1], acc);
    // clmul(a[1], b[1])
    acc = _mm_clmulepi64_si128(a->val[0], b->val[0], 17);
    d[2] = _mm_xor_si128(d[2], acc);

    // aa.val[0] = d[0] + (d[1] <B< 8) + (d[-1] >B> 8)
    aa.val[0] = d[0];
    aa.val[0] = _mm_xor_si128(aa.val[0], _mm_bslli_si128(d[1], 8));
    // aa.val[1] = d[2] + (d[3] <B< 8) + (d[1] >B> 8)
    aa.val[1] = d[2];
    aa.val[1] = _mm_xor_si128(aa.val[1], _mm_bsrli_si128(d[1], 8));
    __m128i tt[1] = {0};
    dfield_elem_t t;
    res->val[0] = _mm_xor_si128(aa.val[0], aa.val[1]);
    t.val[0] = _mm_shuffle_epi32(aa.val[1], 0x4E);
    //* x^7
    tt[0] = _mm_xor_si128(tt[0], _mm_srli_epi64(t.val[0], 57));
    //* x^2
    tt[0] = _mm_xor_si128(tt[0], _mm_srli_epi64(t.val[0], 62));
    //* x^1
    tt[0] = _mm_xor_si128(tt[0], _mm_srli_epi64(t.val[0], 63));
    res->val[0] = _mm_xor_si128(res->val[0], tt[0]);
    aa.val[1] = _mm_xor_si128(
        aa.val[1], _mm_and_si128(_mm_set_epi64x(0, 0xFFFFFFFFFFFFFFFF), tt[0]));
    //* x^7
    res->val[0] = _mm_xor_si128(res->val[0], _mm_slli_epi64(aa.val[1], 7));
    //* x^2
    res->val[0] = _mm_xor_si128(res->val[0], _mm_slli_epi64(aa.val[1], 2));
    //* x^1
    res->val[0] = _mm_xor_si128(res->val[0], _mm_slli_epi64(aa.val[1], 1));
    return 0;
}

static inline __attribute__((always_inline)) int
field_mul_no_carry(dfield_elem_t *res, const field_elem_t *a,
                   const field_elem_t *b)
{
    __m128i acc = {0};
    __m128i d[3] = {0};
    // clmul(a[0], b[0])
    acc = _mm_clmulepi64_si128(a->val[0], b->val[0], 0);
    d[0] = _mm_xor_si128(d[0], acc);
    // clmul(a[0], b[1])
    acc = _mm_clmulepi64_si128(a->val[0], b->val[0], 16);
    d[1] = _mm_xor_si128(d[1], acc);
    // clmul(a[1], b[0])
    acc = _mm_clmulepi64_si128(a->val[0], b->val[0], 1);
    d[1] = _mm_xor_si128(d[1], acc);
    // clmul(a[1], b[1])
    acc = _mm_clmulepi64_si128(a->val[0], b->val[0], 17);
    d[2] = _mm_xor_si128(d[2], acc);

    // res->val[0] = d[0] + (d[1] <B< 8) + (d[-1] >B> 8)
    res->val[0] = d[0];
    res->val[0] = _mm_xor_si128(res->val[0], _mm_bslli_si128(d[1], 8));
    // res->val[1] = d[2] + (d[3] <B< 8) + (d[1] >B> 8)
    res->val[1] = d[2];
    res->val[1] = _mm_xor_si128(res->val[1], _mm_bsrli_si128(d[1], 8));
    return 0;
}

static inline __attribute__((always_inline)) int
field_mul_no_carry2(dfield_elem_t res[2], const field_elem_t a[2],
                   const field_elem_t b[2])
{
    __m128i acc[2] = {0};
    __m128i d[3][2] = {0};
    // clmul(a[0], b[0])
    acc[0] = _mm_clmulepi64_si128(a[0].val[0], b[0].val[0], 0);
    acc[1] = _mm_clmulepi64_si128(a[1].val[0], b[1].val[0], 0);
    d[0][0] = _mm_xor_si128(d[0][0], acc[0]);
    d[0][1] = _mm_xor_si128(d[0][1], acc[1]);
    // clmul(a[0], b[1])
    acc[0] = _mm_clmulepi64_si128(a[0].val[0], b[0].val[0], 16);
    acc[1] = _mm_clmulepi64_si128(a[1].val[0], b[1].val[0], 16);
    d[1][0] = _mm_xor_si128(d[1][0], acc[0]);
    d[1][1] = _mm_xor_si128(d[1][1], acc[1]);
    // clmul(a[1], b[0])
    acc[0] = _mm_clmulepi64_si128(a[0].val[0], b[0].val[0], 1);
    acc[1] = _mm_clmulepi64_si128(a[1].val[0], b[1].val[0], 1);
    d[1][0] = _mm_xor_si128(d[1][0], acc[0]);
    d[1][1] = _mm_xor_si128(d[1][1], acc[1]);
    // clmul(a[1], b[1])
    acc[0] = _mm_clmulepi64_si128(a[0].val[0], b[0].val[0], 17);
    acc[1] = _mm_clmulepi64_si128(a[1].val[0], b[1].val[0], 17);
    d[2][0] = _mm_xor_si128(d[2][0], acc[0]);
    d[2][1] = _mm_xor_si128(d[2][1], acc[1]);

    // res->val[0] = d[0] + (d[1] <B< 8) + (d[-1] >B> 8)
    res[0].val[0] = d[0][0];
    res[1].val[0] = d[0][1];
    res[0].val[0] = _mm_xor_si128(res[0].val[0], _mm_bslli_si128(d[1][0], 8));
    res[1].val[0] = _mm_xor_si128(res[1].val[0], _mm_bslli_si128(d[1][1], 8));
    // res->val[1] = d[2] + (d[3] <B< 8) + (d[1] >B> 8)
    res[0].val[1] = d[2][0];
    res[1].val[1] = d[2][1];
    res[0].val[1] = _mm_xor_si128(res[0].val[1], _mm_bsrli_si128(d[1][0], 8));
    res[1].val[1] = _mm_xor_si128(res[1].val[1], _mm_bsrli_si128(d[1][1], 8));
    return 0;
}

static inline __attribute__((always_inline)) int
field_mul_no_carry8(dfield_elem_t res[8], const field_elem_t a[8],
                   const field_elem_t b[8])
{
    __m128i acc[8] = {0};
    __m128i d[3][8] = {0};
    // clmul(a[0], b[0])
    acc[0] = _mm_clmulepi64_si128(a[0].val[0], b[0].val[0], 0);
    acc[1] = _mm_clmulepi64_si128(a[1].val[0], b[1].val[0], 0);
    acc[2] = _mm_clmulepi64_si128(a[2].val[0], b[2].val[0], 0);
    acc[3] = _mm_clmulepi64_si128(a[3].val[0], b[3].val[0], 0);
    acc[4] = _mm_clmulepi64_si128(a[4].val[0], b[4].val[0], 0);
    acc[5] = _mm_clmulepi64_si128(a[5].val[0], b[5].val[0], 0);
    acc[6] = _mm_clmulepi64_si128(a[6].val[0], b[6].val[0], 0);
    acc[7] = _mm_clmulepi64_si128(a[7].val[0], b[7].val[0], 0);
    d[0][0] = _mm_xor_si128(d[0][0], acc[0]);
    d[0][1] = _mm_xor_si128(d[0][1], acc[1]);
    d[0][2] = _mm_xor_si128(d[0][2], acc[2]);
    d[0][3] = _mm_xor_si128(d[0][3], acc[3]);
    d[0][4] = _mm_xor_si128(d[0][4], acc[4]);
    d[0][5] = _mm_xor_si128(d[0][5], acc[5]);
    d[0][6] = _mm_xor_si128(d[0][6], acc[6]);
    d[0][7] = _mm_xor_si128(d[0][7], acc[7]);
    // clmul(a[0], b[1])
    acc[0] = _mm_clmulepi64_si128(a[0].val[0], b[0].val[0], 16);
    acc[1] = _mm_clmulepi64_si128(a[1].val[0], b[1].val[0], 16);
    acc[2] = _mm_clmulepi64_si128(a[2].val[0], b[2].val[0], 16);
    acc[3] = _mm_clmulepi64_si128(a[3].val[0], b[3].val[0], 16);
    acc[4] = _mm_clmulepi64_si128(a[4].val[0], b[4].val[0], 16);
    acc[5] = _mm_clmulepi64_si128(a[5].val[0], b[5].val[0], 16);
    acc[6] = _mm_clmulepi64_si128(a[6].val[0], b[6].val[0], 16);
    acc[7] = _mm_clmulepi64_si128(a[7].val[0], b[7].val[0], 16);
    d[1][0] = _mm_xor_si128(d[1][0], acc[0]);
    d[1][1] = _mm_xor_si128(d[1][1], acc[1]);
    d[1][2] = _mm_xor_si128(d[1][2], acc[2]);
    d[1][3] = _mm_xor_si128(d[1][3], acc[3]);
    d[1][4] = _mm_xor_si128(d[1][4], acc[4]);
    d[1][5] = _mm_xor_si128(d[1][5], acc[5]);
    d[1][6] = _mm_xor_si128(d[1][6], acc[6]);
    d[1][7] = _mm_xor_si128(d[1][7], acc[7]);
    // clmul(a[1], b[0])
    acc[0] = _mm_clmulepi64_si128(a[0].val[0], b[0].val[0], 1);
    acc[1] = _mm_clmulepi64_si128(a[1].val[0], b[1].val[0], 1);
    acc[2] = _mm_clmulepi64_si128(a[2].val[0], b[2].val[0], 1);
    acc[3] = _mm_clmulepi64_si128(a[3].val[0], b[3].val[0], 1);
    acc[4] = _mm_clmulepi64_si128(a[4].val[0], b[4].val[0], 1);
    acc[5] = _mm_clmulepi64_si128(a[5].val[0], b[5].val[0], 1);
    acc[6] = _mm_clmulepi64_si128(a[6].val[0], b[6].val[0], 1);
    acc[7] = _mm_clmulepi64_si128(a[7].val[0], b[7].val[0], 1);
    d[1][0] = _mm_xor_si128(d[1][0], acc[0]);
    d[1][1] = _mm_xor_si128(d[1][1], acc[1]);
    d[1][2] = _mm_xor_si128(d[1][2], acc[2]);
    d[1][3] = _mm_xor_si128(d[1][3], acc[3]);
    d[1][4] = _mm_xor_si128(d[1][4], acc[4]);
    d[1][5] = _mm_xor_si128(d[1][5], acc[5]);
    d[1][6] = _mm_xor_si128(d[1][6], acc[6]);
    d[1][7] = _mm_xor_si128(d[1][7], acc[7]);
    // clmul(a[1], b[1])
    acc[0] = _mm_clmulepi64_si128(a[0].val[0], b[0].val[0], 17);
    acc[1] = _mm_clmulepi64_si128(a[1].val[0], b[1].val[0], 17);
    acc[2] = _mm_clmulepi64_si128(a[2].val[0], b[2].val[0], 17);
    acc[3] = _mm_clmulepi64_si128(a[3].val[0], b[3].val[0], 17);
    acc[4] = _mm_clmulepi64_si128(a[4].val[0], b[4].val[0], 17);
    acc[5] = _mm_clmulepi64_si128(a[5].val[0], b[5].val[0], 17);
    acc[6] = _mm_clmulepi64_si128(a[6].val[0], b[6].val[0], 17);
    acc[7] = _mm_clmulepi64_si128(a[7].val[0], b[7].val[0], 17);
    d[2][0] = _mm_xor_si128(d[2][0], acc[0]);
    d[2][1] = _mm_xor_si128(d[2][1], acc[1]);
    d[2][2] = _mm_xor_si128(d[2][2], acc[2]);
    d[2][3] = _mm_xor_si128(d[2][3], acc[3]);
    d[2][4] = _mm_xor_si128(d[2][4], acc[4]);
    d[2][5] = _mm_xor_si128(d[2][5], acc[5]);
    d[2][6] = _mm_xor_si128(d[2][6], acc[6]);
    d[2][7] = _mm_xor_si128(d[2][7], acc[7]);

    // res->val[0] = d[0] + (d[1] <B< 8) + (d[-1] >B> 8)
    res[0].val[0] = d[0][0];
    res[1].val[0] = d[0][1];
    res[2].val[0] = d[0][2];
    res[3].val[0] = d[0][3];
    res[4].val[0] = d[0][4];
    res[5].val[0] = d[0][5];
    res[6].val[0] = d[0][6];
    res[7].val[0] = d[0][7];
    res[0].val[0] = _mm_xor_si128(res[0].val[0], _mm_bslli_si128(d[1][0], 8));
    res[1].val[0] = _mm_xor_si128(res[1].val[0], _mm_bslli_si128(d[1][1], 8));
    res[2].val[0] = _mm_xor_si128(res[2].val[0], _mm_bslli_si128(d[1][2], 8));
    res[3].val[0] = _mm_xor_si128(res[3].val[0], _mm_bslli_si128(d[1][3], 8));
    res[4].val[0] = _mm_xor_si128(res[4].val[0], _mm_bslli_si128(d[1][4], 8));
    res[5].val[0] = _mm_xor_si128(res[5].val[0], _mm_bslli_si128(d[1][5], 8));
    res[6].val[0] = _mm_xor_si128(res[6].val[0], _mm_bslli_si128(d[1][6], 8));
    res[7].val[0] = _mm_xor_si128(res[7].val[0], _mm_bslli_si128(d[1][7], 8));
    // res->val[1] = d[2] + (d[3] <B< 8) + (d[1] >B> 8)
    res[0].val[1] = d[2][0];
    res[1].val[1] = d[2][1];
    res[2].val[1] = d[2][2];
    res[3].val[1] = d[2][3];
    res[4].val[1] = d[2][4];
    res[5].val[1] = d[2][5];
    res[6].val[1] = d[2][6];
    res[7].val[1] = d[2][7];
    res[0].val[1] = _mm_xor_si128(res[0].val[1], _mm_bsrli_si128(d[1][0], 8));
    res[1].val[1] = _mm_xor_si128(res[1].val[1], _mm_bsrli_si128(d[1][1], 8));
    res[2].val[1] = _mm_xor_si128(res[2].val[1], _mm_bsrli_si128(d[1][2], 8));
    res[3].val[1] = _mm_xor_si128(res[3].val[1], _mm_bsrli_si128(d[1][3], 8));
    res[4].val[1] = _mm_xor_si128(res[4].val[1], _mm_bsrli_si128(d[1][4], 8));
    res[5].val[1] = _mm_xor_si128(res[5].val[1], _mm_bsrli_si128(d[1][5], 8));
    res[6].val[1] = _mm_xor_si128(res[6].val[1], _mm_bsrli_si128(d[1][6], 8));
    res[7].val[1] = _mm_xor_si128(res[7].val[1], _mm_bsrli_si128(d[1][7], 8));
    return 0;
}

static inline __attribute__((always_inline)) int
field_mul_no_carry8i(dfield_elem_t res[8], const field_elem_t a[8],
                   const field_elem_t b[8])
{
    __m128i acc[8] = {0};
    __m128i d[3][8] = {0};
    // clmul(a[0], b[0])
    acc[0] = _mm_clmulepi64_si128(a[0].val[0], b[7].val[0], 0);
    acc[1] = _mm_clmulepi64_si128(a[1].val[0], b[6].val[0], 0);
    acc[2] = _mm_clmulepi64_si128(a[2].val[0], b[5].val[0], 0);
    acc[3] = _mm_clmulepi64_si128(a[3].val[0], b[4].val[0], 0);
    acc[4] = _mm_clmulepi64_si128(a[4].val[0], b[3].val[0], 0);
    acc[5] = _mm_clmulepi64_si128(a[5].val[0], b[2].val[0], 0);
    acc[6] = _mm_clmulepi64_si128(a[6].val[0], b[1].val[0], 0);
    acc[7] = _mm_clmulepi64_si128(a[7].val[0], b[0].val[0], 0);
    d[0][0] = _mm_xor_si128(d[0][0], acc[0]);
    d[0][1] = _mm_xor_si128(d[0][1], acc[1]);
    d[0][2] = _mm_xor_si128(d[0][2], acc[2]);
    d[0][3] = _mm_xor_si128(d[0][3], acc[3]);
    d[0][4] = _mm_xor_si128(d[0][4], acc[4]);
    d[0][5] = _mm_xor_si128(d[0][5], acc[5]);
    d[0][6] = _mm_xor_si128(d[0][6], acc[6]);
    d[0][7] = _mm_xor_si128(d[0][7], acc[7]);
    // clmul(a[0], b[1])
    acc[0] = _mm_clmulepi64_si128(a[0].val[0], b[7].val[0], 16);
    acc[1] = _mm_clmulepi64_si128(a[1].val[0], b[6].val[0], 16);
    acc[2] = _mm_clmulepi64_si128(a[2].val[0], b[5].val[0], 16);
    acc[3] = _mm_clmulepi64_si128(a[3].val[0], b[4].val[0], 16);
    acc[4] = _mm_clmulepi64_si128(a[4].val[0], b[3].val[0], 16);
    acc[5] = _mm_clmulepi64_si128(a[5].val[0], b[2].val[0], 16);
    acc[6] = _mm_clmulepi64_si128(a[6].val[0], b[1].val[0], 16);
    acc[7] = _mm_clmulepi64_si128(a[7].val[0], b[0].val[0], 16);
    d[1][0] = _mm_xor_si128(d[1][0], acc[0]);
    d[1][1] = _mm_xor_si128(d[1][1], acc[1]);
    d[1][2] = _mm_xor_si128(d[1][2], acc[2]);
    d[1][3] = _mm_xor_si128(d[1][3], acc[3]);
    d[1][4] = _mm_xor_si128(d[1][4], acc[4]);
    d[1][5] = _mm_xor_si128(d[1][5], acc[5]);
    d[1][6] = _mm_xor_si128(d[1][6], acc[6]);
    d[1][7] = _mm_xor_si128(d[1][7], acc[7]);
    // clmul(a[1], b[0])
    acc[0] = _mm_clmulepi64_si128(a[0].val[0], b[7].val[0], 1);
    acc[1] = _mm_clmulepi64_si128(a[1].val[0], b[6].val[0], 1);
    acc[2] = _mm_clmulepi64_si128(a[2].val[0], b[5].val[0], 1);
    acc[3] = _mm_clmulepi64_si128(a[3].val[0], b[4].val[0], 1);
    acc[4] = _mm_clmulepi64_si128(a[4].val[0], b[3].val[0], 1);
    acc[5] = _mm_clmulepi64_si128(a[5].val[0], b[2].val[0], 1);
    acc[6] = _mm_clmulepi64_si128(a[6].val[0], b[1].val[0], 1);
    acc[7] = _mm_clmulepi64_si128(a[7].val[0], b[0].val[0], 1);
    d[1][0] = _mm_xor_si128(d[1][0], acc[0]);
    d[1][1] = _mm_xor_si128(d[1][1], acc[1]);
    d[1][2] = _mm_xor_si128(d[1][2], acc[2]);
    d[1][3] = _mm_xor_si128(d[1][3], acc[3]);
    d[1][4] = _mm_xor_si128(d[1][4], acc[4]);
    d[1][5] = _mm_xor_si128(d[1][5], acc[5]);
    d[1][6] = _mm_xor_si128(d[1][6], acc[6]);
    d[1][7] = _mm_xor_si128(d[1][7], acc[7]);
    // clmul(a[1], b[1])
    acc[0] = _mm_clmulepi64_si128(a[0].val[0], b[7].val[0], 17);
    acc[1] = _mm_clmulepi64_si128(a[1].val[0], b[6].val[0], 17);
    acc[2] = _mm_clmulepi64_si128(a[2].val[0], b[5].val[0], 17);
    acc[3] = _mm_clmulepi64_si128(a[3].val[0], b[4].val[0], 17);
    acc[4] = _mm_clmulepi64_si128(a[4].val[0], b[3].val[0], 17);
    acc[5] = _mm_clmulepi64_si128(a[5].val[0], b[2].val[0], 17);
    acc[6] = _mm_clmulepi64_si128(a[6].val[0], b[1].val[0], 17);
    acc[7] = _mm_clmulepi64_si128(a[7].val[0], b[0].val[0], 17);
    d[2][0] = _mm_xor_si128(d[2][0], acc[0]);
    d[2][1] = _mm_xor_si128(d[2][1], acc[1]);
    d[2][2] = _mm_xor_si128(d[2][2], acc[2]);
    d[2][3] = _mm_xor_si128(d[2][3], acc[3]);
    d[2][4] = _mm_xor_si128(d[2][4], acc[4]);
    d[2][5] = _mm_xor_si128(d[2][5], acc[5]);
    d[2][6] = _mm_xor_si128(d[2][6], acc[6]);
    d[2][7] = _mm_xor_si128(d[2][7], acc[7]);

    // res->val[0] = d[0] + (d[1] <B< 8) + (d[-1] >B> 8)
    res[0].val[0] = d[0][0];
    res[1].val[0] = d[0][1];
    res[2].val[0] = d[0][2];
    res[3].val[0] = d[0][3];
    res[4].val[0] = d[0][4];
    res[5].val[0] = d[0][5];
    res[6].val[0] = d[0][6];
    res[7].val[0] = d[0][7];
    res[0].val[0] = _mm_xor_si128(res[0].val[0], _mm_bslli_si128(d[1][0], 8));
    res[1].val[0] = _mm_xor_si128(res[1].val[0], _mm_bslli_si128(d[1][1], 8));
    res[2].val[0] = _mm_xor_si128(res[2].val[0], _mm_bslli_si128(d[1][2], 8));
    res[3].val[0] = _mm_xor_si128(res[3].val[0], _mm_bslli_si128(d[1][3], 8));
    res[4].val[0] = _mm_xor_si128(res[4].val[0], _mm_bslli_si128(d[1][4], 8));
    res[5].val[0] = _mm_xor_si128(res[5].val[0], _mm_bslli_si128(d[1][5], 8));
    res[6].val[0] = _mm_xor_si128(res[6].val[0], _mm_bslli_si128(d[1][6], 8));
    res[7].val[0] = _mm_xor_si128(res[7].val[0], _mm_bslli_si128(d[1][7], 8));
    // res->val[1] = d[2] + (d[3] <B< 8) + (d[1] >B> 8)
    res[0].val[1] = d[2][0];
    res[1].val[1] = d[2][1];
    res[2].val[1] = d[2][2];
    res[3].val[1] = d[2][3];
    res[4].val[1] = d[2][4];
    res[5].val[1] = d[2][5];
    res[6].val[1] = d[2][6];
    res[7].val[1] = d[2][7];
    res[0].val[1] = _mm_xor_si128(res[0].val[1], _mm_bsrli_si128(d[1][0], 8));
    res[1].val[1] = _mm_xor_si128(res[1].val[1], _mm_bsrli_si128(d[1][1], 8));
    res[2].val[1] = _mm_xor_si128(res[2].val[1], _mm_bsrli_si128(d[1][2], 8));
    res[3].val[1] = _mm_xor_si128(res[3].val[1], _mm_bsrli_si128(d[1][3], 8));
    res[4].val[1] = _mm_xor_si128(res[4].val[1], _mm_bsrli_si128(d[1][4], 8));
    res[5].val[1] = _mm_xor_si128(res[5].val[1], _mm_bsrli_si128(d[1][5], 8));
    res[6].val[1] = _mm_xor_si128(res[6].val[1], _mm_bsrli_si128(d[1][6], 8));
    res[7].val[1] = _mm_xor_si128(res[7].val[1], _mm_bsrli_si128(d[1][7], 8));
    return 0;
}

static inline __attribute__((always_inline)) int
field_mul_reduce(field_elem_t *res, const field_elem_t *a,
                 const field_elem_t *b)
{
    field_mul(res, a, b);
    return 0;
}

static inline __attribute__((always_inline)) int
field_sqr(field_elem_t *res, const field_elem_t *a)
{
    dfield_elem_t aa;
    // clmul(a[0], a[0])
    aa.val[0] = _mm_clmulepi64_si128(a->val[0], a->val[0], 0);
    // clmul(a[1], a[1])
    aa.val[1] = _mm_clmulepi64_si128(a->val[0], a->val[0], 17);
    __m128i tt[1] = {0};
    dfield_elem_t t;
    res->val[0] = _mm_xor_si128(aa.val[0], aa.val[1]);
    t.val[0] = _mm_shuffle_epi32(aa.val[1], 0x4E);
    //* x^7
    tt[0] = _mm_xor_si128(tt[0], _mm_srli_epi64(t.val[0], 57));
    //* x^2
    tt[0] = _mm_xor_si128(tt[0], _mm_srli_epi64(t.val[0], 62));
    //* x^1
    tt[0] = _mm_xor_si128(tt[0], _mm_srli_epi64(t.val[0], 63));
    res->val[0] = _mm_xor_si128(res->val[0], tt[0]);
    aa.val[1] = _mm_xor_si128(
        aa.val[1], _mm_and_si128(_mm_set_epi64x(0, 0xFFFFFFFFFFFFFFFF), tt[0]));
    //* x^7
    res->val[0] = _mm_xor_si128(res->val[0], _mm_slli_epi64(aa.val[1], 7));
    //* x^2
    res->val[0] = _mm_xor_si128(res->val[0], _mm_slli_epi64(aa.val[1], 2));
    //* x^1
    res->val[0] = _mm_xor_si128(res->val[0], _mm_slli_epi64(aa.val[1], 1));
    return 0;
}

static inline __attribute__((always_inline)) int
field_sqr_no_carry(dfield_elem_t *res, const field_elem_t *a)
{
    // clmul(a[0], a[0])
    res->val[0] = _mm_clmulepi64_si128(a->val[0], a->val[0], 0);
    // clmul(a[1], a[1])
    res->val[1] = _mm_clmulepi64_si128(a->val[0], a->val[0], 17);
    return 0;
}

static inline __attribute__((always_inline)) int
field_sqr_reduce(field_elem_t *res, const field_elem_t *a)
{
    field_sqr(res, a);
    return 0;
}

static inline __attribute__((always_inline)) int
field_add(field_elem_t *res, const field_elem_t *a, const field_elem_t *b)
{
    res->val[0] = _mm_xor_si128(a->val[0], b->val[0]);
    return 0;
}

static inline __attribute__((always_inline)) int
field_add_reduce(field_elem_t *res, const field_elem_t *a,
                 const field_elem_t *b)
{
    field_add(res, a, b);
    return 0;
}

static inline __attribute__((always_inline)) int
field_add_mix(dfield_elem_t *res, const dfield_elem_t *a, const field_elem_t *b)
{
    res->val[0] = _mm_xor_si128(a->val[0], b->val[0]);
    res->val[1] = a->val[1];
    return 0;
}

static inline __attribute__((always_inline)) int
field_add_dbl(dfield_elem_t *res, const dfield_elem_t *a,
              const dfield_elem_t *b)
{
    res->val[0] = _mm_xor_si128(a->val[0], b->val[0]);
    res->val[1] = _mm_xor_si128(a->val[1], b->val[1]);
    return 0;
}

static inline __attribute__((always_inline)) int
pack_field_elem(uint8_t *res, const field_elem_t *a)
{
    memcpy(res, a, 16);
    return 0;
}

static inline __attribute__((always_inline)) int
unpack_field_elem(field_elem_t *res, const uint8_t *a)
{
    memcpy(res, a, 16);
    return 0;
}

static inline __attribute__((always_inline)) int unpack_key(field_elem_t *res,
                                                            const uint8_t *a)
{
    memcpy(res, a, 16);
    return 0;
}

static inline __attribute__((always_inline)) int
unpack_and_encode_key(field_elem_t *res, const uint8_t *a)
{
    memcpy(res, a, 16);
    return 0;
}

static inline __attribute__((always_inline)) int
unpack_and_encode_field_elem(field_elem_t *res, const uint8_t *a)
{
    unpack_field_elem(res, a);
    return 0;
}

// static inline __attribute__((always_inline)) int
// unpack_and_encode_last_field_elem(
//     field_elem_t* res,
//     const uint8_t* a,
//     size_t a_size)
// {
//     uint8_t buff[BUFFSIZE] = {0};
//     transform_msg(buff, BUFFSIZE, (uint8_t*) a, a_size);
//     unpack_field_elem(res, (baseint_t *) buff);
//     return 0;
// }

#define UNPACK_AND_ENCODE_LAST_FIELD_ELEM(out, in, inlen)                      \
    if (last) {                                                                \
        unpack_and_encode_last_field_elem(out, in, inlen);                     \
    } else {                                                                   \
        unpack_and_encode_field_elem(out, in);                                 \
    }

static inline __attribute__((always_inline)) field_elem_t
field_elem_get_one(void)
{
    return (field_elem_t){{{1, 0}}};
}

#define GET_MACRO(_1, _2, _3, NAME, ...) NAME
#define NOT_PRECOMPUTED(name) name
#define PRECOMPUTED(name) name
#define DECLARE_PC_ELEM(name) field_elem_t NOT_PRECOMPUTED(name);
#define DECLARE_PC_ELEM_ARRAY(name, size)                                      \
    field_elem_t NOT_PRECOMPUTED(name)[size]
#define UNPACK_PC_FIELD_ELEM_SINGLE(name, buff)                                \
    unpack_field_elem(&NOT_PRECOMPUTED(name), (baseint_t *)buff);
#define UNPACK_PC_FIELD_ELEM_ARRAY(name, buff, index)                          \
    unpack_field_elem(&NOT_PRECOMPUTED(name)[index], (baseint_t *)buff)
#define UNPACK_PC_FIELD_ELEM(...)                                              \
    GET_MACRO(__VA_ARGS__, UNPACK_PC_FIELD_ELEM_ARRAY,                         \
              UNPACK_PC_FIELD_ELEM_SINGLE)                                     \
    (__VA_ARGS__)
#define UNPACK_AND_ENCODE_PC_KEY_SINGLE(name, buff)                            \
    unpack_and_encode_key(&NOT_PRECOMPUTED(name), (baseint_t *)buff);
#define UNPACK_AND_ENCODE_PC_KEY_ARRAY(name, buff, index)                      \
    unpack_and_encode_key(&NOT_PRECOMPUTED(name)[index], (baseint_t *)buff)
#define UNPACK_AND_ENCODE_PC_KEY(...)                                          \
    GET_MACRO(__VA_ARGS__, UNPACK_AND_ENCODE_PC_KEY_ARRAY,                     \
              UNPACK_AND_ENCODE_PC_KEY_SINGLE)                                 \
    (__VA_ARGS__)
#define INIT_PC_KEY(dst, src)
#define FIELD_MUL_PC(dest, srcA, srcB) field_mul(dest, srcA, srcB);
#define FIELD_MUL_PC_NO_CARRY(dest, srcA, srcB)                                \
    field_mul_no_carry(dest, srcA, srcB);
#define FIELD_MUL_PC_REDUCE(dest, srcA, srcB)                                  \
    field_mul_reduce(dest, srcA, srcB);
#define FIELD_SQR_PC(dest, srcA, srcB) field_sqr(dest, srcA, srcB);
#define FIELD_SQR_PC_NO_CARRY(dest, srcA, srcB)                                \
    field_sqr_no_carry(dest, srcA, srcB);
#define FIELD_SQR_PC_REDUCE(dest, srcA, srcB)                                  \
    field_sqr_reduce(dest, srcA, srcB);
