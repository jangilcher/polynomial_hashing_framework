#pragma once
#include <inttypes.h>
#include <stddef.h>
#include <string.h>
#include <immintrin.h>
#include <string.h>

typedef uint64_t baseint_t;
typedef struct field_elem_single field_elem_t;
typedef struct field_elem_double dfield_elem_t;

static inline __attribute__((always_inline)) int carry_round(
    field_elem_t* res,
    dfield_elem_t* a);

static inline __attribute__((always_inline)) int _carry_round(
    field_elem_t* res,
    field_elem_t* a);

static inline __attribute__((always_inline)) int reduce(
    field_elem_t* res,
    const field_elem_t* a);

static inline __attribute__((always_inline)) int field_mul(
    field_elem_t* res,
    const field_elem_t* a,
    const field_elem_t* b);

static inline __attribute__((always_inline)) int field_mul_no_carry(
    dfield_elem_t* res,
    const field_elem_t* a,
    const field_elem_t* b);

static inline __attribute__((always_inline)) int field_mul_reduce(
    field_elem_t* res,
    const field_elem_t* a,
    const field_elem_t* b);

static inline __attribute__((always_inline)) int field_sqr(
    field_elem_t* res,
    const field_elem_t* a);

static inline __attribute__((always_inline)) int field_sqr_no_carry(
    dfield_elem_t* res,
    const field_elem_t* a);

static inline __attribute__((always_inline)) int field_sqr_reduce(
    field_elem_t* res,
    const field_elem_t* a);

static inline __attribute__((always_inline)) int field_add(
    field_elem_t* res,
    const field_elem_t* a,
    const field_elem_t* b);

static inline __attribute__((always_inline)) int field_add_reduce(
    field_elem_t* res,
    const field_elem_t* a,
    const field_elem_t* b);

static inline __attribute__((always_inline)) int field_add_mix(
    dfield_elem_t* res,
    const dfield_elem_t* a,
    const field_elem_t* b);

static inline __attribute__((always_inline)) int field_add_dbl(
    dfield_elem_t* res,
    const dfield_elem_t* a,
    const dfield_elem_t* b);

static inline __attribute__((always_inline)) int pack_field_elem(
    uint8_t* res,
    const field_elem_t* a);

static inline __attribute__((always_inline)) int unpack_field_elem(
    field_elem_t* res,
    const uint8_t* a);

static inline __attribute__((always_inline)) int unpack_key(
    field_elem_t* res,
    const uint8_t* a);

static inline __attribute__((always_inline)) int unpack_and_encode_key(
    field_elem_t* res,
    const uint8_t* a);

// static inline __attribute__((always_inline)) int unpack_and_encode_field_elem(
//     field_elem_t* res,
//     const uint64_t* a)
// {
//     uint8_t buff[BUFFSIZE] = {0};
//     transform_msg(buff, BUFFSIZE, (uint8_t*) a, BLOCKSIZE);
//     unpack_field_elem(res, (baseint_t *) buff);
//     return 0;
// }

// static inline __attribute__((always_inline)) int unpack_and_encode_last_field_elem(
//     field_elem_t* res,
//     const uint64_t* a,
//     size_t a_size)
// {
//     uint8_t buff[BUFFSIZE] = {0};
//     transform_msg(buff, BUFFSIZE, (uint8_t*) a, a_size);
//     unpack_field_elem(res, (baseint_t *) buff);
//     return 0;
// }

// static inline __attribute__((always_inline)) field_elem_t field_elem_get_one(void);
