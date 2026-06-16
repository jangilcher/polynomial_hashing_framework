#pragma once
#include <immintrin.h>
#include <inttypes.h>
#include <stddef.h>
#include <string.h>
#define BUFFSIZE 32
#define DOUBLE_WORDSIZE 128
#define LIMBMASK ((((uint64_t)1) << LIMBBITS) - 1)
#define LIMBMASK2 ((((uint64_t)1) << LIMBBITS2) - 1)
typedef uint64_t baseint_t;
typedef struct field_elem_single {
    __m128i val[2];
} field_elem_t;
typedef struct field_elem_double {
    __m128i val[4];
} dfield_elem_t;

#ifdef DEBUG
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#define PRINT_VECTOR(...) print_vector(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#define PRINT_VECTOR(...)
#endif

#define PRINT_FIELD_ELEM(elem)                                                 \
    DEBUG_PRINTF(                                                              \
        #elem "\n%.16llx %.16llx\n%.16llx %.16llx\n",                          \
        _mm_extract_epi64(elem.val[0], 1), _mm_extract_epi64(elem.val[0], 0),  \
        _mm_extract_epi64(elem.val[1], 1), _mm_extract_epi64(elem.val[1], 0));

#define PRINT_DFIELD_ELEM(elem)                                                \
    DEBUG_PRINTF(                                                              \
        #elem "\n%.16llx %.16llx\n%.16llx %.16llx\n%.16llx %.16llx\n%.16llx "  \
              "%.16llx\n",                                                     \
        _mm_extract_epi64(elem.val[0], 1), _mm_extract_epi64(elem.val[0], 0),  \
        _mm_extract_epi64(elem.val[1], 1), _mm_extract_epi64(elem.val[1], 0),  \
        _mm_extract_epi64(elem.val[2], 1), _mm_extract_epi64(elem.val[2], 0),  \
        _mm_extract_epi64(elem.val[3], 1), _mm_extract_epi64(elem.val[3], 0));

#define HERE DEBUG_PRINTF("%s:%d\n", __FILE__, __LINE__);

// static inline __attribute__((always_inline)) int carry_round(field_elem_t *res,
//                                                              dfield_elem_t *a)
// {
//     __m128i tt[2] = {0};
//     dfield_elem_t t;
//     res->val[0] = _mm_xor_si128(a->val[0], a->val[2]);
//     res->val[1] = _mm_xor_si128(a->val[1], a->val[3]);
//     t.val[0] = _mm_shuffle_epi32(a->val[2], 0x4E);
//     t.val[1] = _mm_unpacklo_epi64(t.val[0], a->val[3]);
//     t.val[0] = _mm_unpackhi_epi64(a->val[3], t.val[0]);
//     //* x^10
//     tt[0] = _mm_xor_si128(tt[0], _mm_srli_epi64(t.val[0], 54));
//     //* x^5
//     tt[0] = _mm_xor_si128(tt[0], _mm_srli_epi64(t.val[0], 59));
//     //* x^2
//     tt[0] = _mm_xor_si128(tt[0], _mm_srli_epi64(t.val[0], 62));
//     res->val[0] = _mm_xor_si128(res->val[0], tt[0]);
//     //* x^10
//     tt[1] = _mm_xor_si128(tt[1], _mm_srli_epi64(t.val[1], 54));
//     //* x^5
//     tt[1] = _mm_xor_si128(tt[1], _mm_srli_epi64(t.val[1], 59));
//     //* x^2
//     tt[1] = _mm_xor_si128(tt[1], _mm_srli_epi64(t.val[1], 62));
//     res->val[1] = _mm_xor_si128(res->val[1], tt[1]);
//     a->val[2] = _mm_xor_si128(
//         a->val[2], _mm_and_si128(_mm_set_epi64x(0, 0xFFFFFFFFFFFFFFFF), tt[0]));
//     //* x^10
//     res->val[0] = _mm_xor_si128(res->val[0], _mm_slli_epi64(a->val[2], 10));
//     res->val[1] = _mm_xor_si128(res->val[1], _mm_slli_epi64(a->val[3], 10));
//     //* x^5
//     res->val[0] = _mm_xor_si128(res->val[0], _mm_slli_epi64(a->val[2], 5));
//     res->val[1] = _mm_xor_si128(res->val[1], _mm_slli_epi64(a->val[3], 5));
//     //* x^2
//     res->val[0] = _mm_xor_si128(res->val[0], _mm_slli_epi64(a->val[2], 2));
//     res->val[1] = _mm_xor_si128(res->val[1], _mm_slli_epi64(a->val[3], 2));
//     return 0;
// }

typedef union {
    __m128i v;
    uint64_t i[2];
    uint32_t i32[4];
    uint8_t  i8[16];
} pvector;

static inline __attribute__((always_inline)) int carry_round(field_elem_t *res,
                                                             dfield_elem_t *a)
{
    __m128i tmp[5];
    __m128i t1, t0;

    const pvector poly = {.i = {0x425, 0}};

    tmp[3] = _mm_clmulepi64_si128(a->val[3], poly.v, 1); // x7
    tmp[2] = _mm_clmulepi64_si128(a->val[3], poly.v, 0); // x6
    tmp[1] = _mm_clmulepi64_si128(a->val[2], poly.v, 1); // x5
    tmp[0] = _mm_clmulepi64_si128(a->val[2], poly.v, 0); // x4
    tmp[4] = _mm_clmulepi64_si128(tmp[3], poly.v, 1); 
    t1 = (__m128i) _mm_shuffle_ps((__m128)tmp[1], (__m128)tmp[3], 0x4E);
    t0 = (__m128i) _mm_shuffle_ps((__m128)tmp[4], (__m128)tmp[1], 0x44);
    res->val[1] = a->val[1] ^ tmp[2];
    res->val[0] = a->val[0] ^ tmp[0];
    res->val[1] ^= t1;
    res->val[0] ^= t0;

    return 0;
}

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
    __m128i d[7] = {0};
    // clmul(a[0], b[0])
    acc = _mm_clmulepi64_si128(a->val[0], b->val[0], 0);
    d[0] = _mm_xor_si128(d[0], acc);
    // clmul(a[0], b[1])
    acc = _mm_clmulepi64_si128(a->val[0], b->val[0], 16);
    d[1] = _mm_xor_si128(d[1], acc);
    // clmul(a[0], b[2])
    acc = _mm_clmulepi64_si128(a->val[0], b->val[1], 0);
    d[2] = _mm_xor_si128(d[2], acc);
    // clmul(a[0], b[3])
    acc = _mm_clmulepi64_si128(a->val[0], b->val[1], 16);
    d[3] = _mm_xor_si128(d[3], acc);
    // clmul(a[1], b[0])
    acc = _mm_clmulepi64_si128(a->val[0], b->val[0], 1);
    d[1] = _mm_xor_si128(d[1], acc);
    // clmul(a[1], b[1])
    acc = _mm_clmulepi64_si128(a->val[0], b->val[0], 17);
    d[2] = _mm_xor_si128(d[2], acc);
    // clmul(a[1], b[2])
    acc = _mm_clmulepi64_si128(a->val[0], b->val[1], 1);
    d[3] = _mm_xor_si128(d[3], acc);
    // clmul(a[1], b[3])
    acc = _mm_clmulepi64_si128(a->val[0], b->val[1], 17);
    d[4] = _mm_xor_si128(d[4], acc);
    // clmul(a[2], b[0])
    acc = _mm_clmulepi64_si128(a->val[1], b->val[0], 0);
    d[2] = _mm_xor_si128(d[2], acc);
    // clmul(a[2], b[1])
    acc = _mm_clmulepi64_si128(a->val[1], b->val[0], 16);
    d[3] = _mm_xor_si128(d[3], acc);
    // clmul(a[2], b[2])
    acc = _mm_clmulepi64_si128(a->val[1], b->val[1], 0);
    d[4] = _mm_xor_si128(d[4], acc);
    // clmul(a[2], b[3])
    acc = _mm_clmulepi64_si128(a->val[1], b->val[1], 16);
    d[5] = _mm_xor_si128(d[5], acc);
    // clmul(a[3], b[0])
    acc = _mm_clmulepi64_si128(a->val[1], b->val[0], 1);
    d[3] = _mm_xor_si128(d[3], acc);
    // clmul(a[3], b[1])
    acc = _mm_clmulepi64_si128(a->val[1], b->val[0], 17);
    d[4] = _mm_xor_si128(d[4], acc);
    // clmul(a[3], b[2])
    acc = _mm_clmulepi64_si128(a->val[1], b->val[1], 1);
    d[5] = _mm_xor_si128(d[5], acc);
    // clmul(a[3], b[3])
    acc = _mm_clmulepi64_si128(a->val[1], b->val[1], 17);
    d[6] = _mm_xor_si128(d[6], acc);

    // aa.val[0] = d[0] + (d[1] <B< 8) + (d[-1] >B> 8)
    aa.val[0] = d[0];
    aa.val[0] = _mm_xor_si128(aa.val[0], _mm_bslli_si128(d[1], 8));
    // aa.val[1] = d[2] + (d[3] <B< 8) + (d[1] >B> 8)
    aa.val[1] = d[2];
    aa.val[1] = _mm_xor_si128(aa.val[1], _mm_bslli_si128(d[3], 8));
    aa.val[1] = _mm_xor_si128(aa.val[1], _mm_bsrli_si128(d[1], 8));
    // aa.val[2] = d[4] + (d[5] <B< 8) + (d[3] >B> 8)
    aa.val[2] = d[4];
    aa.val[2] = _mm_xor_si128(aa.val[2], _mm_bslli_si128(d[5], 8));
    aa.val[2] = _mm_xor_si128(aa.val[2], _mm_bsrli_si128(d[3], 8));
    // aa.val[3] = d[6] + (d[7] <B< 8) + (d[5] >B> 8)
    aa.val[3] = d[6];
    aa.val[3] = _mm_xor_si128(aa.val[3], _mm_bsrli_si128(d[5], 8));
    __m128i tt[2] = {0};
    dfield_elem_t t;
    res->val[0] = _mm_xor_si128(aa.val[0], aa.val[2]);
    res->val[1] = _mm_xor_si128(aa.val[1], aa.val[3]);
    t.val[0] = _mm_shuffle_epi32(aa.val[2], 0x4E);
    t.val[1] = _mm_unpacklo_epi64(t.val[0], aa.val[3]);
    t.val[0] = _mm_unpackhi_epi64(aa.val[3], t.val[0]);
    //* x^10
    tt[0] = _mm_xor_si128(tt[0], _mm_srli_epi64(t.val[0], 54));
    //* x^5
    tt[0] = _mm_xor_si128(tt[0], _mm_srli_epi64(t.val[0], 59));
    //* x^2
    tt[0] = _mm_xor_si128(tt[0], _mm_srli_epi64(t.val[0], 62));
    res->val[0] = _mm_xor_si128(res->val[0], tt[0]);
    //* x^10
    tt[1] = _mm_xor_si128(tt[1], _mm_srli_epi64(t.val[1], 54));
    //* x^5
    tt[1] = _mm_xor_si128(tt[1], _mm_srli_epi64(t.val[1], 59));
    //* x^2
    tt[1] = _mm_xor_si128(tt[1], _mm_srli_epi64(t.val[1], 62));
    res->val[1] = _mm_xor_si128(res->val[1], tt[1]);
    aa.val[2] = _mm_xor_si128(
        aa.val[2], _mm_and_si128(_mm_set_epi64x(0, 0xFFFFFFFFFFFFFFFF), tt[0]));
    //* x^10
    res->val[0] = _mm_xor_si128(res->val[0], _mm_slli_epi64(aa.val[2], 10));
    res->val[1] = _mm_xor_si128(res->val[1], _mm_slli_epi64(aa.val[3], 10));
    //* x^5
    res->val[0] = _mm_xor_si128(res->val[0], _mm_slli_epi64(aa.val[2], 5));
    res->val[1] = _mm_xor_si128(res->val[1], _mm_slli_epi64(aa.val[3], 5));
    //* x^2
    res->val[0] = _mm_xor_si128(res->val[0], _mm_slli_epi64(aa.val[2], 2));
    res->val[1] = _mm_xor_si128(res->val[1], _mm_slli_epi64(aa.val[3], 2));
    return 0;
}

static inline __attribute__((always_inline)) int
field_mul_no_carry(dfield_elem_t *res, const field_elem_t *a,
                   const field_elem_t *b)
{
    __m128i acc = {0};
    __m128i d[7] = {0};
    // clmul(a[0], b[0])
    acc = _mm_clmulepi64_si128(a->val[0], b->val[0], 0);
    d[0] = _mm_xor_si128(d[0], acc);
    // clmul(a[0], b[1])
    acc = _mm_clmulepi64_si128(a->val[0], b->val[0], 16);
    d[1] = _mm_xor_si128(d[1], acc);
    // clmul(a[0], b[2])
    acc = _mm_clmulepi64_si128(a->val[0], b->val[1], 0);
    d[2] = _mm_xor_si128(d[2], acc);
    // clmul(a[0], b[3])
    acc = _mm_clmulepi64_si128(a->val[0], b->val[1], 16);
    d[3] = _mm_xor_si128(d[3], acc);
    // clmul(a[1], b[0])
    acc = _mm_clmulepi64_si128(a->val[0], b->val[0], 1);
    d[1] = _mm_xor_si128(d[1], acc);
    // clmul(a[1], b[1])
    acc = _mm_clmulepi64_si128(a->val[0], b->val[0], 17);
    d[2] = _mm_xor_si128(d[2], acc);
    // clmul(a[1], b[2])
    acc = _mm_clmulepi64_si128(a->val[0], b->val[1], 1);
    d[3] = _mm_xor_si128(d[3], acc);
    // clmul(a[1], b[3])
    acc = _mm_clmulepi64_si128(a->val[0], b->val[1], 17);
    d[4] = _mm_xor_si128(d[4], acc);
    // clmul(a[2], b[0])
    acc = _mm_clmulepi64_si128(a->val[1], b->val[0], 0);
    d[2] = _mm_xor_si128(d[2], acc);
    // clmul(a[2], b[1])
    acc = _mm_clmulepi64_si128(a->val[1], b->val[0], 16);
    d[3] = _mm_xor_si128(d[3], acc);
    // clmul(a[2], b[2])
    acc = _mm_clmulepi64_si128(a->val[1], b->val[1], 0);
    d[4] = _mm_xor_si128(d[4], acc);
    // clmul(a[2], b[3])
    acc = _mm_clmulepi64_si128(a->val[1], b->val[1], 16);
    d[5] = _mm_xor_si128(d[5], acc);
    // clmul(a[3], b[0])
    acc = _mm_clmulepi64_si128(a->val[1], b->val[0], 1);
    d[3] = _mm_xor_si128(d[3], acc);
    // clmul(a[3], b[1])
    acc = _mm_clmulepi64_si128(a->val[1], b->val[0], 17);
    d[4] = _mm_xor_si128(d[4], acc);
    // clmul(a[3], b[2])
    acc = _mm_clmulepi64_si128(a->val[1], b->val[1], 1);
    d[5] = _mm_xor_si128(d[5], acc);
    // clmul(a[3], b[3])
    acc = _mm_clmulepi64_si128(a->val[1], b->val[1], 17);
    d[6] = _mm_xor_si128(d[6], acc);

    // res->val[0] = d[0] + (d[1] <B< 8) + (d[-1] >B> 8)
    res->val[0] = d[0];
    res->val[0] = _mm_xor_si128(res->val[0], _mm_bslli_si128(d[1], 8));
    // res->val[1] = d[2] + (d[3] <B< 8) + (d[1] >B> 8)
    res->val[1] = d[2];
    res->val[1] = _mm_xor_si128(res->val[1], _mm_bslli_si128(d[3], 8));
    res->val[1] = _mm_xor_si128(res->val[1], _mm_bsrli_si128(d[1], 8));
    // res->val[2] = d[4] + (d[5] <B< 8) + (d[3] >B> 8)
    res->val[2] = d[4];
    res->val[2] = _mm_xor_si128(res->val[2], _mm_bslli_si128(d[5], 8));
    res->val[2] = _mm_xor_si128(res->val[2], _mm_bsrli_si128(d[3], 8));
    // res->val[3] = d[6] + (d[7] <B< 8) + (d[5] >B> 8)
    res->val[3] = d[6];
    res->val[3] = _mm_xor_si128(res->val[3], _mm_bsrli_si128(d[5], 8));
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
    // clmul(a[2], a[2])
    aa.val[2] = _mm_clmulepi64_si128(a->val[1], a->val[1], 0);
    // clmul(a[3], a[3])
    aa.val[3] = _mm_clmulepi64_si128(a->val[1], a->val[1], 17);
    __m128i tt[2] = {0};
    dfield_elem_t t;
    res->val[0] = _mm_xor_si128(aa.val[0], aa.val[2]);
    res->val[1] = _mm_xor_si128(aa.val[1], aa.val[3]);
    t.val[0] = _mm_shuffle_epi32(aa.val[2], 0x4E);
    t.val[1] = _mm_unpacklo_epi64(t.val[0], aa.val[3]);
    t.val[0] = _mm_unpackhi_epi64(aa.val[3], t.val[0]);
    //* x^10
    tt[0] = _mm_xor_si128(tt[0], _mm_srli_epi64(t.val[0], 54));
    //* x^5
    tt[0] = _mm_xor_si128(tt[0], _mm_srli_epi64(t.val[0], 59));
    //* x^2
    tt[0] = _mm_xor_si128(tt[0], _mm_srli_epi64(t.val[0], 62));
    res->val[0] = _mm_xor_si128(res->val[0], tt[0]);
    //* x^10
    tt[1] = _mm_xor_si128(tt[1], _mm_srli_epi64(t.val[1], 54));
    //* x^5
    tt[1] = _mm_xor_si128(tt[1], _mm_srli_epi64(t.val[1], 59));
    //* x^2
    tt[1] = _mm_xor_si128(tt[1], _mm_srli_epi64(t.val[1], 62));
    res->val[1] = _mm_xor_si128(res->val[1], tt[1]);
    aa.val[2] = _mm_xor_si128(
        aa.val[2], _mm_and_si128(_mm_set_epi64x(0, 0xFFFFFFFFFFFFFFFF), tt[0]));
    //* x^10
    res->val[0] = _mm_xor_si128(res->val[0], _mm_slli_epi64(aa.val[2], 10));
    res->val[1] = _mm_xor_si128(res->val[1], _mm_slli_epi64(aa.val[3], 10));
    //* x^5
    res->val[0] = _mm_xor_si128(res->val[0], _mm_slli_epi64(aa.val[2], 5));
    res->val[1] = _mm_xor_si128(res->val[1], _mm_slli_epi64(aa.val[3], 5));
    //* x^2
    res->val[0] = _mm_xor_si128(res->val[0], _mm_slli_epi64(aa.val[2], 2));
    res->val[1] = _mm_xor_si128(res->val[1], _mm_slli_epi64(aa.val[3], 2));
    return 0;
}

static inline __attribute__((always_inline)) int
field_sqr_no_carry(dfield_elem_t *res, const field_elem_t *a)
{
    // clmul(a[0], a[0])
    res->val[0] = _mm_clmulepi64_si128(a->val[0], a->val[0], 0);
    // clmul(a[1], a[1])
    res->val[1] = _mm_clmulepi64_si128(a->val[0], a->val[0], 17);
    // clmul(a[2], a[2])
    res->val[2] = _mm_clmulepi64_si128(a->val[1], a->val[1], 0);
    // clmul(a[3], a[3])
    res->val[3] = _mm_clmulepi64_si128(a->val[1], a->val[1], 17);
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
    res->val[1] = _mm_xor_si128(a->val[1], b->val[1]);
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
    res->val[1] = _mm_xor_si128(a->val[1], b->val[1]);
    res->val[2] = a->val[2];
    res->val[3] = a->val[3];
    return 0;
}

static inline __attribute__((always_inline)) int
field_add_dbl(dfield_elem_t *res, const dfield_elem_t *a,
              const dfield_elem_t *b)
{
    res->val[0] = _mm_xor_si128(a->val[0], b->val[0]);
    res->val[1] = _mm_xor_si128(a->val[1], b->val[1]);
    res->val[2] = _mm_xor_si128(a->val[2], b->val[2]);
    res->val[3] = _mm_xor_si128(a->val[3], b->val[3]);
    return 0;
}

static inline __attribute__((always_inline)) int
pack_field_elem(uint8_t *res, const field_elem_t *a)
{
    memcpy(res, a, 32);
    return 0;
}

static inline __attribute__((always_inline)) int
unpack_field_elem(field_elem_t *res, const uint8_t *a)
{
    memcpy(res, a, 32);
    return 0;
}

static inline __attribute__((always_inline)) int unpack_key(field_elem_t *res,
                                                            const uint8_t *a)
{
    memcpy(res, a, 32);
    return 0;
}

static inline __attribute__((always_inline)) int
unpack_and_encode_key(field_elem_t *res, const uint8_t *a)
{
    memcpy(res, a, 32);
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
    return (field_elem_t){{{1, 0}, {0, 0}}};
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

// inline void add(vector res[2], const vector a[2], const vector b[2]) {
//     res[0].v = _mm_xor_si128(a[0].v, b[0].v);
//     res[1].v = _mm_xor_si128(a[1].v, b[1].v);
// }

// inline void add_double(vector res[4], const vector a[4], const vector b[4]) {
//     res[0].v = a[0].v ^ b[0].v;
//     res[1].v = a[1].v ^ b[1].v;
//     res[2].v = a[2].v ^ b[2].v;
//     res[3].v = a[3].v ^ b[3].v;
// }

// inline void mul_reduce(vector res[2], const vector a[2], const vector b[2]) {
//     __m128i acc;
//     __m128i acc00;
//     __m128i acc01;
//     __m128i acc02;
//     __m128i acc03;
//     __m128i acc10;
//     __m128i acc11;
//     __m128i acc12;
//     __m128i acc13;
//     __m128i acc20;
//     __m128i acc21;
//     __m128i acc22;
//     __m128i acc23;
//     __m128i acc30;
//     __m128i acc31;
//     __m128i acc32;
//     __m128i acc33;
//     __m128i d[7] = {0};
//     __m128i res0;
//     __m128i res1;
//     __m128i res2;
//     __m128i res3;

//     // clmul(a[0], b[0])
//     acc00 = _mm_clmulepi64_si128(a[0].v, b[0].v, 0);
//     d[0] = acc00;
//     // clmul(a[0], b[1])
//     acc01 = _mm_clmulepi64_si128(a[0].v, b[0].v, 16);
//     d[1] = acc01;
//     // clmul(a[1], b[0])
//     acc10 = _mm_clmulepi64_si128(a[0].v, b[0].v, 1);
//     d[1] = _mm_xor_si128(d[1], acc10);
//     // clmul(a[0], b[2])
//     acc02 = _mm_clmulepi64_si128(a[0].v, b[1].v, 0);
//     d[2] = acc02;
//     // clmul(a[1], b[1])
//     acc11 = _mm_clmulepi64_si128(a[0].v, b[0].v, 17);
//     d[2] = _mm_xor_si128(d[2], acc11);
//     // clmul(a[2], b[0])
//     acc20 = _mm_clmulepi64_si128(a[1].v, b[0].v, 0);
//     d[2] = _mm_xor_si128(d[2], acc20);
//     // clmul(a[0], b[3])
//     acc03 = _mm_clmulepi64_si128(a[0].v, b[1].v, 16);
//     d[3] = acc03;
//     // clmul(a[1], b[2])
//     acc12 = _mm_clmulepi64_si128(a[0].v, b[1].v, 1);
//     d[3] = _mm_xor_si128(d[3], acc12);
//     // clmul(a[2], b[1])
//     acc21 = _mm_clmulepi64_si128(a[1].v, b[0].v, 16);
//     d[3] = _mm_xor_si128(d[3], acc21);
//     // clmul(a[3], b[0])
//     acc30 = _mm_clmulepi64_si128(a[1].v, b[0].v, 1);
//     d[3] = _mm_xor_si128(d[3], acc30);
//     // clmul(a[1], b[3])
//     acc13 = _mm_clmulepi64_si128(a[0].v, b[1].v, 17);
//     d[4] = acc13;
//     // clmul(a[2], b[2])
//     acc22 = _mm_clmulepi64_si128(a[1].v, b[1].v, 0);
//     d[4] = _mm_xor_si128(d[4], acc22);
//     // clmul(a[3], b[1])
//     acc31 = _mm_clmulepi64_si128(a[1].v, b[0].v, 17);
//     d[4] = _mm_xor_si128(d[4], acc31);
//     // clmul(a[2], b[3])
//     acc23 = _mm_clmulepi64_si128(a[1].v, b[1].v, 16);
//     d[5] = acc23;
//     // clmul(a[3], b[2])
//     acc32 = _mm_clmulepi64_si128(a[1].v, b[1].v, 1);
//     d[5] = _mm_xor_si128(d[5], acc32);
//     // clmul(a[3], b[3])
//     acc33 = _mm_clmulepi64_si128(a[1].v, b[1].v, 17);
//     d[6] = acc33;

//     // res_low = d[0] + (d[1] <B< 8) + (d[-1] >B> 8)
//     res0 = _mm_slli_si128(d[1], 8);
//     res0 = _mm_xor_si128(res0, d[0]);
//     // res_high = d[2] + (d[3] <B< 8) + (d[1] >B> 8)
//     res1 = _mm_slli_si128(d[3], 8);
//     acc = _mm_srli_si128(d[1], 8);
//     res1 = _mm_xor_si128(res1, d[2]);
//     res1 = _mm_xor_si128(res1, acc);
//     // res->val[2] = d[4] + (d[5] <B< 8) + (d[3] >B> 8)
//     res2 = _mm_slli_si128(d[5], 8);
//     acc = _mm_srli_si128(d[3], 8);
//     res2 = _mm_xor_si128(res2, d[4]);
//     res2 = _mm_xor_si128(res2, acc);
//     // res->val[3] = d[6] + (d[7] <B< 8) + (d[5] >B> 8)
//     res3 = _mm_srli_si128(d[5], 8);
//     res3 = _mm_xor_si128(res3, d[6]);

//     __m128i r_tmp[5];
//     __m128i poly = polynomial.v;
//     __m128i r_res0, r_res1, r_t1, r_t0;

//     r_tmp[3] = _mm_clmulepi64_si128(res3, poly, 1); // x7
//     r_tmp[2] = _mm_clmulepi64_si128(res3, poly, 0); // x6
//     r_tmp[1] = _mm_clmulepi64_si128(res2, poly, 1); // x5
//     r_tmp[0] = _mm_clmulepi64_si128(res2, poly, 0); // x4
//     r_tmp[4] = _mm_clmulepi64_si128(r_tmp[3], poly, 1);
//     r_t1 = (__m128i)_mm_shuffle_ps((__m128)r_tmp[1], (__m128)r_tmp[3], 0x4E);
//     r_t0 = (__m128i)_mm_shuffle_ps((__m128)r_tmp[4], (__m128)r_tmp[1], 0x44);
//     r_res1 = _mm_xor_si128(res1, r_tmp[2]);
//     r_res0 = _mm_xor_si128(res0, r_tmp[0]);
//     res[1].v = _mm_xor_si128(r_res1, r_t1);
//     res[0].v = _mm_xor_si128(r_res0, r_t0);

//     return;
// }

// inline void mul(vector res[4], const vector a[2], const vector b[2]) {
//     __m128i acc;
//     __m128i acc00;
//     __m128i acc01;
//     __m128i acc02;
//     __m128i acc03;
//     __m128i acc10;
//     __m128i acc11;
//     __m128i acc12;
//     __m128i acc13;
//     __m128i acc20;
//     __m128i acc21;
//     __m128i acc22;
//     __m128i acc23;
//     __m128i acc30;
//     __m128i acc31;
//     __m128i acc32;
//     __m128i acc33;
//     __m128i d[7] = {0};

//     // clmul(a[0], b[0])
//     acc00 = _mm_clmulepi64_si128(a[0].v, b[0].v, 0);
//     d[0] = acc00;
//     // clmul(a[0], b[1])
//     acc01 = _mm_clmulepi64_si128(a[0].v, b[0].v, 16);
//     d[1] = acc01;
//     // clmul(a[1], b[0])
//     acc10 = _mm_clmulepi64_si128(a[0].v, b[0].v, 1);
//     d[1] = _mm_xor_si128(d[1], acc10);
//     // clmul(a[0], b[2])
//     acc02 = _mm_clmulepi64_si128(a[0].v, b[1].v, 0);
//     d[2] = acc02;
//     // clmul(a[1], b[1])
//     acc11 = _mm_clmulepi64_si128(a[0].v, b[0].v, 17);
//     d[2] = _mm_xor_si128(d[2], acc11);
//     // clmul(a[2], b[0])
//     acc20 = _mm_clmulepi64_si128(a[1].v, b[0].v, 0);
//     d[2] = _mm_xor_si128(d[2], acc20);
//     // clmul(a[0], b[3])
//     acc03 = _mm_clmulepi64_si128(a[0].v, b[1].v, 16);
//     d[3] = acc03;
//     // clmul(a[1], b[2])
//     acc12 = _mm_clmulepi64_si128(a[0].v, b[1].v, 1);
//     d[3] = _mm_xor_si128(d[3], acc12);
//     // clmul(a[2], b[1])
//     acc21 = _mm_clmulepi64_si128(a[1].v, b[0].v, 16);
//     d[3] = _mm_xor_si128(d[3], acc21);
//     // clmul(a[3], b[0])
//     acc30 = _mm_clmulepi64_si128(a[1].v, b[0].v, 1);
//     d[3] = _mm_xor_si128(d[3], acc30);
//     // clmul(a[1], b[3])
//     acc13 = _mm_clmulepi64_si128(a[0].v, b[1].v, 17);
//     d[4] = acc13;
//     // clmul(a[2], b[2])
//     acc22 = _mm_clmulepi64_si128(a[1].v, b[1].v, 0);
//     d[4] = _mm_xor_si128(d[4], acc22);
//     // clmul(a[3], b[1])
//     acc31 = _mm_clmulepi64_si128(a[1].v, b[0].v, 17);
//     d[4] = _mm_xor_si128(d[4], acc31);
//     // clmul(a[2], b[3])
//     acc23 = _mm_clmulepi64_si128(a[1].v, b[1].v, 16);
//     d[5] = acc23;
//     // clmul(a[3], b[2])
//     acc32 = _mm_clmulepi64_si128(a[1].v, b[1].v, 1);
//     d[5] = _mm_xor_si128(d[5], acc32);
//     // clmul(a[3], b[3])
//     acc33 = _mm_clmulepi64_si128(a[1].v, b[1].v, 17);
//     d[6] = acc33;

//     // res_low = d[0] + (d[1] <B< 8) + (d[-1] >B> 8)
//     res[0].v = _mm_slli_si128(d[1], 8);
//     res[0].v = _mm_xor_si128(res[0].v, d[0]);
//     // res_high = d[2] + (d[3] <B< 8) + (d[1] >B> 8)
//     res[1].v = _mm_slli_si128(d[3], 8);
//     acc = _mm_srli_si128(d[1], 8);
//     res[1].v = _mm_xor_si128(res[1].v, d[2]);
//     res[1].v = _mm_xor_si128(res[1].v, acc);
//     // res->val[2] = d[4] + (d[5] <B< 8) + (d[3] >B> 8)
//     res[2].v = _mm_slli_si128(d[5], 8);
//     acc = _mm_srli_si128(d[3], 8);
//     res[2].v = _mm_xor_si128(res[2].v, d[4]);
//     res[2].v = _mm_xor_si128(res[2].v, acc);
//     // res->val[3] = d[6] + (d[7] <B< 8) + (d[5] >B> 8)
//     res[3].v = _mm_srli_si128(d[5], 8);
//     res[3].v = _mm_xor_si128(res[3].v, d[6]);

//     return;
// }

// inline void reduce(vector res[2], const vector in[4]) {
//     __m128i r_tmp[5];
//     __m128i poly = polynomial.v;
//     __m128i r_res0, r_res1, r_t1, r_t0;

//     r_tmp[3] = _mm_clmulepi64_si128(in[3].v, poly, 1); // x7
//     r_tmp[2] = _mm_clmulepi64_si128(in[3].v, poly, 0); // x6
//     r_tmp[1] = _mm_clmulepi64_si128(in[2].v, poly, 1); // x5
//     r_tmp[0] = _mm_clmulepi64_si128(in[2].v, poly, 0); // x4
//     r_tmp[4] = _mm_clmulepi64_si128(r_tmp[3], poly, 1);
//     r_t1 = (__m128i)_mm_shuffle_ps((__m128)r_tmp[1], (__m128)r_tmp[3], 0x4E);
//     r_t0 = (__m128i)_mm_shuffle_ps((__m128)r_tmp[4], (__m128)r_tmp[1], 0x44);
//     r_res1 = _mm_xor_si128(in[1].v, r_tmp[2]);
//     r_res0 = _mm_xor_si128(in[0].v, r_tmp[0]);
//     res[1].v = _mm_xor_si128(r_res1, r_t1);
//     res[0].v = _mm_xor_si128(r_res0, r_t0);

//     return;
// }
