// MIT License
//
// Copyright (c) 2023 Jan Gilcher, Jérôme Govinden
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef field_arithmetic_key_clamping_H_
#define field_arithmetic_key_clamping_H_

#include "../field_arithmetic/field_arithmetic.h"

#define LAST_MSG_BLOCKSIZE (8 * BLOCKSIZE - 128)
#define LAST_FIELDELEM_BLOCKSIZE (PI - 128)

#ifndef OUTER_PARAM0
#define OUTER_PARAM0 64
#endif
#define WORDSIZE OUTER_PARAM0

#ifndef OUTER_PARAM1
#define OUTER_PARAM1 3
#endif

#ifndef OUTER_PARAM2
#define OUTER_PARAM2 1
#endif

#ifndef OUTER_PARAM3
#define OUTER_PARAM3 0
#endif

// OUTER_PARAM0 = wordsize either 32 of 64

// OUTER_PARAM1 = nb of limbs

// OUTER_PARAM2 = encoding
// if OUTER_PARAM2 = 0 then no encoding
// if OUTER_PARAM2 = 1 then append bit

// OUTER_PARAM3 = field multiplication (+ associated precomputation)
// if OUTER_PARAM3 = 0 then uses a mixed of low and upper bit key clamping as
// poly1305 if OUTER_PARAM3 = 1 then uses a mixed of low and upper bit key
// clamping as poly1305 (technique described in blog post) if OUTER_PARAM3 = 2
// then uses full low and upper bit key clamping if OUTER_PARAM3 = 3 then uses
// upper bit key clamping with 2 limbs key if OUTER_PARAM3 = 4 then uses upper
// bit key clamping with 3 limbs key

static inline __attribute__((always_inline)) void
unpack_field_elem_KC(field_elem_t *res, uint64_t *a) {
    res->val[0] = a[0];
    res->val[1] = a[1];
    res->val[2] = a[2] & ((((uint64_t)1) << LAST_MSG_BLOCKSIZE) - 1);
    res->val[2] |= (uint64_t)0x0 << LAST_MSG_BLOCKSIZE;
}

static inline __attribute__((always_inline)) void
unpack_and_encode_field_elem_KC(field_elem_t *res, uint64_t *a) {
    res->val[0] = a[0];
    res->val[1] = a[1];
    res->val[2] = a[2] & ((((uint64_t)1) << LAST_MSG_BLOCKSIZE) - 1);
    res->val[2] |= (uint64_t)OUTER_PARAM2 << LAST_MSG_BLOCKSIZE;
}

static inline __attribute__((always_inline)) void
unpack_and_encode_last_field_elem_KC(field_elem_t *res, uint64_t *a,
                                     size_t size) {
    uint64_t tmp[BUFFSIZE] = {0};
    memcpy(tmp, a, size);
    ((uint8_t *)tmp)[size] = (uint8_t)OUTER_PARAM2;
    res->val[0] = tmp[0];
    res->val[1] = tmp[1];
    res->val[2] = tmp[2];
}

static inline __attribute__((always_inline)) void
pack_field_elem_KC(uint64_t *res, field_elem_t *a) {
    res[0] = (a->val[0]);
    res[1] = (a->val[1]);
    res[2] = (a->val[2]);
}

#if OUTER_PARAM3 == 0
// if OUTER_PARAM3 = 0 then uses a mixed of low and upper bit key clamping as
// poly1305
static inline __attribute__((always_inline)) void
precompute_factor_KC(field_elem_precomputed_t *res, field_elem_t *b) {
    res->val[0][0][0] = b->val[0];
    res->val[0][1][1] = b->val[1];
    res->val[1][1][1] =
        ((uint64_t)(b->val[1] >> LAST_FIELDELEM_BLOCKSIZE) * DELTA);
}
#elif OUTER_PARAM3 <= 2
// if OUTER_PARAM3 = 1 then uses a mixed of low and upper bit key clamping as
// poly1305 (technique described in blog post) if OUTER_PARAM3 = 2 then uses
// full low and upper bit key clamping
static inline __attribute__((always_inline)) void
precompute_factor_KC(field_elem_precomputed_t *res, field_elem_t *b) {
    res->val[0][0][0] = b->val[0];
    res->val[0][1][1] = b->val[1];

    res->val[1][0][0] =
        ((uint64_t)(b->val[0] >> LAST_FIELDELEM_BLOCKSIZE) * DELTA);
    res->val[1][1][1] =
        ((uint64_t)(b->val[1] >> LAST_FIELDELEM_BLOCKSIZE) * DELTA);
}
#elif OUTER_PARAM3 == 3
// if OUTER_PARAM3 = 3 then uses upper bit key clamping with 2 limbs key
static inline __attribute__((always_inline)) void
precompute_factor_KC(field_elem_precomputed_t *res, field_elem_t *b) {
    res->val[0][0][0] = b->val[0];
    res->val[0][1][1] = b->val[1];
    res->val[1][1][1] = ((uint64_t)b->val[1] * DELTA);
}
#elif OUTER_PARAM3 == 4
// if OUTER_PARAM3 = 4 then uses upper bit key clamping with 3 limbs key
static inline __attribute__((always_inline)) void
precompute_factor_KC(field_elem_precomputed_t *res, field_elem_t *b) {
    res->val[0][0][0] = b->val[0];
    res->val[0][1][1] = b->val[1];
    res->val[0][2][2] = b->val[2];
    res->val[1][1][1] = ((uint64_t)b->val[1] * DELTA);
    res->val[1][2][2] = ((uint64_t)b->val[2] * DELTA);
}
#endif

#if OUTER_PARAM3 == 0
// if OUTER_PARAM3 = 0 then uses a mixed of low and upper bit key clamping as
// poly1305
// static inline __attribute__((always_inline)) void
// field_mul_precomputed_no_carry_KC(
//    dfield_elem_t* res,
//    field_elem_t* a,
//    field_elem_precomputed_t* b)
//{
//    res->val[0] = ((uint128_t) a->val[0] * b->val[0][0][0]);
//    res->val[0] += (uint128_t) a->val[1] * b->val[1][1][1];
//
//    res->val[1] = ((uint128_t) a->val[1] * b->val[0][0][0]);
//    res->val[1] += ((uint128_t) a->val[0] * b->val[0][1][1]);
//    res->val[1] += ((uint64_t) a->val[2] * b->val[1][1][1]);
//
//    res->val[2] = ((uint64_t) a->val[2] * b->val[0][0][0]);
//}
static inline __attribute__((always_inline)) void
field_mul_precomputed_no_carry_KC(dfield_elem_t *res, field_elem_t *a,
                                  field_elem_precomputed_t *b) {
    res->val[0] = ((uint128_t)a->val[0] * b->val[0][0][0]);
    res->val[0] += (uint128_t)a->val[1] * b->val[1][1][1];

    res->val[1] = ((uint128_t)a->val[1] * b->val[0][0][0]);
    res->val[1] += ((uint128_t)a->val[0] * b->val[0][1][1]);
    res->val[1] += ((uint128_t)a->val[2] * b->val[1][1][1]);

    res->val[2] = ((uint128_t)a->val[2] * b->val[0][0][0]);
}
#elif OUTER_PARAM3 == 1
// if OUTER_PARAM3 = 1 then uses a mixed of low and upper bit key clamping as
// poly1305 (technique described in blog post)
static inline __attribute__((always_inline)) void
field_mul_precomputed_no_carry_KC(dfield_elem_t *res, field_elem_t *a,
                                  field_elem_precomputed_t *b) {
    res->val[0] = ((uint128_t)a->val[0] * b->val[0][0][0]);
    res->val[0] += (uint128_t)a->val[1] * b->val[1][1][1];
    res->val[0] += ((uint64_t)a->val[2] * b->val[1][0][0]);

    res->val[1] = ((uint128_t)a->val[1] * b->val[0][0][0]);
    res->val[1] += ((uint128_t)a->val[0] * b->val[0][1][1]);
    res->val[1] += ((uint64_t)a->val[2] * b->val[1][1][1]);

    res->val[2] = ((uint64_t)a->val[2] *
                   (b->val[0][0][0] &
                    (((((uint64_t)1) << LAST_FIELDELEM_BLOCKSIZE) - 1))));
}
#elif OUTER_PARAM3 == 2
// if OUTER_PARAM3 = 2 then uses full low and upper bit key clamping
// static inline __attribute__((always_inline)) void
// field_mul_precomputed_no_carry_KC(
//    dfield_elem_t* res,
//    field_elem_t* a,
//    field_elem_precomputed_t* b)
//{
//    res->val[0] = ((uint128_t) a->val[0] * b->val[0][0][0]);
//    res->val[0] += (uint128_t) a->val[1] * b->val[1][1][1];
//    res->val[0] += ((uint64_t) a->val[2] * b->val[1][0][0]);
//
//    res->val[1] = ((uint128_t) a->val[1] * b->val[0][0][0]);
//    res->val[1] += ((uint128_t) a->val[0] * b->val[0][1][1]);
//    res->val[1] += ((uint64_t) a->val[2] * b->val[1][1][1]);
//
//    res->val[2] = 0;
//}
static inline __attribute__((always_inline)) void
field_mul_precomputed_no_carry_KC(dfield_elem_t *res, field_elem_t *a,
                                  field_elem_precomputed_t *b) {
    res->val[0] = ((uint128_t)a->val[0] * b->val[0][0][0]);
    res->val[0] += (uint128_t)a->val[1] * b->val[1][1][1];
    res->val[0] += ((uint128_t)a->val[2] * b->val[1][0][0]);

    res->val[1] = ((uint128_t)a->val[1] * b->val[0][0][0]);
    res->val[1] += ((uint128_t)a->val[0] * b->val[0][1][1]);
    res->val[1] += ((uint128_t)a->val[2] * b->val[1][1][1]);

    res->val[2] = 0;
}
#elif OUTER_PARAM3 == 3
// if OUTER_PARAM3 = 3 then uses upper bit key clamping with 2 limbs key
static inline __attribute__((always_inline)) void
field_mul_precomputed_no_carry_KC(dfield_elem_t *res, field_elem_t *a,
                                  field_elem_precomputed_t *b) {
    uint128_t acc;
    acc = ((uint128_t)a->val[2] * b->val[1][1][1]);

    res->val[0] = ((uint128_t)a->val[0] * b->val[0][0][0]);
    res->val[0] +=
        (((uint64_t)acc) & (((((uint64_t)1) << LAST_FIELDELEM_BLOCKSIZE) - 1)))
        << (WORDSIZE - LAST_FIELDELEM_BLOCKSIZE);

    res->val[1] = ((uint128_t)a->val[1] * b->val[0][0][0]);
    res->val[1] += ((uint128_t)a->val[0] * b->val[0][1][1]);
    res->val[1] += (uint128_t)((uint128_t)acc >> LAST_FIELDELEM_BLOCKSIZE);

    res->val[2] = ((uint128_t)a->val[2] * b->val[0][0][0]);
    res->val[2] += ((uint128_t)a->val[1] * b->val[0][1][1]);
}
#elif OUTER_PARAM3 == 4
// if OUTER_PARAM3 = 4 then uses upper bit key clamping with 3 limbs key
static inline __attribute__((always_inline)) void
field_mul_precomputed_no_carry_KC(dfield_elem_t *res, field_elem_t *a,
                                  field_elem_precomputed_t *b) {
    uint128_t acc;
    acc = ((uint128_t)a->val[2] * b->val[1][1][1]);
    acc += ((uint128_t)a->val[1] * b->val[1][2][2]);

    res->val[0] = ((uint128_t)a->val[0] * b->val[0][0][0]);
    res->val[0] +=
        (((uint64_t)acc) & (((((uint64_t)1) << LAST_FIELDELEM_BLOCKSIZE) - 1)))
        << (WORDSIZE - LAST_FIELDELEM_BLOCKSIZE);

    res->val[1] = ((uint128_t)a->val[1] * b->val[0][0][0]);
    res->val[1] += ((uint128_t)a->val[0] * b->val[0][1][1]);
    res->val[1] += (uint128_t)((uint128_t)acc >> LAST_FIELDELEM_BLOCKSIZE);

    acc = ((uint128_t)a->val[2] * b->val[1][2][2]);
    res->val[1] +=
        (((uint64_t)acc) & (((((uint64_t)1) << LAST_FIELDELEM_BLOCKSIZE) - 1)))
        << (WORDSIZE - LAST_FIELDELEM_BLOCKSIZE);

    res->val[2] = ((uint128_t)a->val[2] * b->val[0][0][0]);
    res->val[2] += ((uint128_t)a->val[1] * b->val[0][1][1]);
    res->val[2] += ((uint128_t)a->val[0] * b->val[0][2][2]);
    res->val[2] += (uint128_t)((uint128_t)acc >> LAST_FIELDELEM_BLOCKSIZE);
}
#endif

#if (OUTER_PARAM3 == 0) || (OUTER_PARAM3 == 3)
// if OUTER_PARAM3 = 0 then uses a mixed of low and upper bit key clamping as
// poly1305 if OUTER_PARAM3 = 3 then uses upper bit key clamping with 2 limbs
// key
static inline __attribute__((always_inline)) void
carry_round_KC(field_elem_t *res, dfield_elem_t *a) {
    // need to check how many rounds are necessary
    // a->val[2] += (uint64_t) (a->val[1] >> (64));
    // a->val[1] = ((uint64_t) (a->val[1])) ;

    a->val[0] += ((uint128_t)(a->val[2] >> LAST_FIELDELEM_BLOCKSIZE) *
                  DELTA); // 64 vs 128 for up
    res->val[2] = ((uint64_t)(a->val[2])) &
                  (((((uint64_t)1) << LAST_FIELDELEM_BLOCKSIZE) - 1));

    a->val[1] += (uint64_t)(a->val[0] >> (64));
    res->val[0] = ((uint64_t)(a->val[0]));

    res->val[2] += (uint64_t)(a->val[1] >> (64));
    res->val[1] = ((uint64_t)(a->val[1]));
}
#elif OUTER_PARAM3 == 1
// if OUTER_PARAM3 = 1 then uses a mixed of low and upper bit key clamping as
// poly1305 (technique described in blog post)
static inline __attribute__((always_inline)) void
carry_round_KC(field_elem_t *res, dfield_elem_t *a) {
    // need to check how many rounds are necessary
    a->val[2] += (uint64_t)(a->val[1] >> (64));
    a->val[1] = ((uint64_t)(a->val[1]));

    a->val[0] += ((uint64_t)(a->val[2] >> LAST_FIELDELEM_BLOCKSIZE) *
                  DELTA); // 64 vs 128 for up
    res->val[2] = ((uint64_t)(a->val[2])) &
                  (((((uint64_t)1) << LAST_FIELDELEM_BLOCKSIZE) - 1));

    a->val[1] += (uint64_t)(a->val[0] >> (64));
    res->val[0] = ((uint64_t)(a->val[0]));

    res->val[2] += (uint64_t)(a->val[1] >> (64));
    res->val[1] = ((uint64_t)(a->val[1]));
}
#elif OUTER_PARAM3 == 2
// if OUTER_PARAM3 = 2 then uses full low and upper bit key clamping
static inline __attribute__((always_inline)) void
carry_round_KC(field_elem_t *res, dfield_elem_t *a) {
    // a->val[0] += ((uint64_t) (a->val[2] >> LAST_FIELDELEM_BLOCKSIZE)* DELTA);
    // // 64 vs 128 for up
    res->val[2] = ((uint64_t)(a->val[2]));

    a->val[1] += (uint64_t)(a->val[0] >> (64));
    res->val[0] = ((uint64_t)(a->val[0]));

    res->val[2] += (uint64_t)(a->val[1] >> (64));
    res->val[1] = ((uint64_t)(a->val[1]));
}
#elif OUTER_PARAM3 == 4
// if OUTER_PARAM3 = 4 then uses upper bit key clamping with 3 limbs key
static inline __attribute__((always_inline)) void
carry_round_KC(field_elem_t *res, dfield_elem_t *a) {
    // need to check how many rounds are necessary
    a->val[2] += (uint64_t)(a->val[1] >> (64));
    a->val[1] = ((uint64_t)(a->val[1]));

    a->val[0] += ((uint128_t)(a->val[2] >> LAST_FIELDELEM_BLOCKSIZE) *
                  DELTA); // 64 vs 128 for up
    res->val[2] = ((uint64_t)(a->val[2])) &
                  (((((uint64_t)1) << LAST_FIELDELEM_BLOCKSIZE) - 1));

    a->val[1] += (uint64_t)(a->val[0] >> (64));
    res->val[0] = ((uint64_t)(a->val[0]));

    res->val[2] += (uint64_t)(a->val[1] >> (64));
    res->val[1] = ((uint64_t)(a->val[1]));
}
#endif

#if (OUTER_PARAM3 == 0) || (OUTER_PARAM3 == 2) || (OUTER_PARAM3 == 3)
// if OUTER_PARAM3 = 0 then uses a mixed of low and upper bit key clamping as
// poly1305 if OUTER_PARAM3 = 2 then uses full low and upper bit key clamping if
// OUTER_PARAM3 = 3 then uses upper bit key clamping with 2 limbs key
static inline __attribute__((always_inline)) void
carry_round_KC_final(field_elem_t *res, dfield_elem_t *a) {
    // need to check how many rounds are necessary
    a->val[2] += (uint64_t)(a->val[1] >> (64));
    a->val[1] = ((uint64_t)(a->val[1]));

    a->val[0] += ((uint128_t)(a->val[2] >> LAST_FIELDELEM_BLOCKSIZE) *
                  DELTA); // 64 vs 128 for up
    res->val[2] = ((uint64_t)(a->val[2])) &
                  (((((uint64_t)1) << LAST_FIELDELEM_BLOCKSIZE) - 1));

    a->val[1] += (uint64_t)(a->val[0] >> (64));
    res->val[0] = ((uint64_t)(a->val[0]));

    res->val[2] += (uint64_t)(a->val[1] >> (64));
    res->val[1] = ((uint64_t)(a->val[1]));
}
#elif OUTER_PARAM3 == 1
// if OUTER_PARAM3 = 1 then uses a mixed of low and upper bit key clamping as
// poly1305 (technique described in blog post)
// similar as carry_round_KC
static inline __attribute__((always_inline)) void
carry_round_KC_final(field_elem_t *res, dfield_elem_t *a) {
    // need to check how many rounds are necessary
    a->val[2] += (uint64_t)(a->val[1] >> (64));
    a->val[1] = ((uint64_t)(a->val[1]));

    a->val[0] += ((uint64_t)(a->val[2] >> LAST_FIELDELEM_BLOCKSIZE) *
                  DELTA); // 64 vs 128 for up
    res->val[2] = ((uint64_t)(a->val[2])) &
                  (((((uint64_t)1) << LAST_FIELDELEM_BLOCKSIZE) - 1));

    a->val[1] += (uint64_t)(a->val[0] >> (64));
    res->val[0] = ((uint64_t)(a->val[0]));

    res->val[2] += (uint64_t)(a->val[1] >> (64));
    res->val[1] = ((uint64_t)(a->val[1]));
}
#endif

static inline __attribute__((always_inline)) int reduce_KC(field_elem_t *res,
                                                           field_elem_t *a) {
    field_elem_t t;
    uint128_t td;
    uint64_t c;
    uint64_t mask;
    td = a->val[0] + DELTA;
    c = (uint64_t)(td >> (64));
    t.val[0] = ((uint64_t)(td));
    td = a->val[1] + c;
    c = (uint64_t)(td >> (64));
    t.val[1] = ((uint64_t)(td));
    t.val[2] = a->val[2] + c;
    t.val[2] += -(((uint64_t)1) << LAST_FIELDELEM_BLOCKSIZE);
    mask = (uint64_t)(t.val[2] >> (63));
    mask += -1;
    t.val[0] = (t.val[0]) & (mask);
    t.val[1] = (t.val[1]) & (mask);
    t.val[2] = (t.val[2]) & (mask);
    mask = ~mask;
    res->val[0] = ((a->val[0]) & (mask)) | (t.val[0]);
    res->val[1] = ((a->val[1]) & (mask)) | (t.val[1]);
    res->val[2] = ((a->val[2]) & (mask)) | (t.val[2]);
    return 0;
}

// copy from old code

static inline __attribute__((always_inline)) void
unpack_field_elem_KC_1305(field_elem_t *res, uint64_t *a) {
    res->val[0] = a[0];
    res->val[1] = a[1];
    res->val[2] = 0;
}

static inline __attribute__((always_inline)) void
unpack_field_elem_KC_1503(field_elem_t *res, uint64_t *a) {
    res->val[0] = a[0];
    res->val[1] = a[1];
    res->val[2] = a[2] & ((((uint64_t)1) << 16) - 1);
}

static inline __attribute__((always_inline)) void
unpack_field_elem_KC_1743(field_elem_t *res, uint64_t *a) {
    res->val[0] = a[0];
    res->val[1] = a[1];
    res->val[2] = a[2] & ((((uint64_t)1) << 40) - 1);
}

static inline __attribute__((always_inline)) void
unpack_last_field_elem_KC(field_elem_t *res, uint64_t *a, size_t size) {
    uint64_t tmp[BUFFSIZE] = {0};
    memcpy(tmp, a, size);
    res->val[0] = tmp[0];
    res->val[1] = tmp[1];
    res->val[2] = tmp[2];
}

static inline __attribute__((always_inline)) void
precompute_factor_KC_1305(field_elem_precomputed_t *res, field_elem_t *b) {
    res->val[0][0][0] = b->val[0];
    res->val[0][1][1] = b->val[1];

    //    res->val[1][0][0] =(b->val[0] >> 2 )* 5;
    res->val[1][1][0] = (b->val[1] >> 2) * 5;
}

static inline __attribute__((always_inline)) void
precompute_factor_KC_1503(field_elem_precomputed_t *res, field_elem_t *b) {
    res->val[0][0][0] = b->val[0];
    res->val[0][1][1] = b->val[1];

    //    res->val[1][0][0] =(b->val[0] >> 2 )* 5;
    res->val[1][1][0] = (b->val[1]) * 3;
}

static inline __attribute__((always_inline)) void
precompute_factor_KC_1743(field_elem_precomputed_t *res, field_elem_t *b) {
    res->val[0][0][0] = b->val[0];
    res->val[0][1][1] = b->val[1];

    //    res->val[1][0][0] =(b->val[0] >> 2 )* 5;
    res->val[1][1][0] = (b->val[1]) * 3;
}

static inline __attribute__((always_inline)) void
field_mul_precomputed_no_carry_KC_1305(dfield_elem_t *res, field_elem_t *a,
                                       field_elem_precomputed_t *b) {
    uint128_t acc;

    acc = ((uint128_t)a->val[0] * b->val[0][0][0]);
    res->val[0] = acc;
    // acc = (uint128_t) a->val[2] * b->val[1][0][0] + (uint128_t) a->val[1] *
    // b->val[1][1][0];
    acc = (uint128_t)a->val[1] * b->val[1][1][0];
    res->val[0] += acc;

    acc = ((uint128_t)a->val[1] * b->val[0][0][0]);
    res->val[1] = acc;
    acc = ((uint128_t)a->val[0] * b->val[0][1][1]);
    res->val[1] += acc;
    acc = (a->val[2] * b->val[1][1][0]);
    res->val[1] += acc;

    acc = (a->val[2] * b->val[0][0][0]);
    res->val[2] = acc;
    //    acc = ((uint128_t) a->val[1] * b->val[0][1][1]);
    //    res->val[2] += acc;
}

static inline __attribute__((always_inline)) void
field_mul_precomputed_no_carry_KC_1503(dfield_elem_t *res, field_elem_t *a,
                                       field_elem_precomputed_t *b) {
    uint128_t acc;

    res->val[0] = ((uint128_t)a->val[0] * b->val[0][0][0]);

    res->val[1] = ((uint128_t)a->val[1] * b->val[0][0][0]);
    res->val[1] += ((uint128_t)a->val[0] * b->val[0][1][1]);

    res->val[2] = ((uint128_t)a->val[2] * b->val[0][0][0]);
    res->val[2] += ((uint128_t)a->val[1] * b->val[0][1][1]);

    acc = ((uint128_t)a->val[2] * b->val[1][1][0]);

    res->val[0] += (((uint64_t)acc) & (((((uint64_t)1) << 22) - 1))) << 42;
    res->val[1] += (uint128_t)(acc >> 22);
}

static inline __attribute__((always_inline)) void
field_mul_precomputed_no_carry_KC_1743(dfield_elem_t *res, field_elem_t *a,
                                       field_elem_precomputed_t *b) {
    uint128_t acc;

    acc = ((uint128_t)a->val[0] * b->val[0][0][0]);
    res->val[0] = acc;
    // acc = (uint128_t) a->val[2] * b->val[1][0][0] + (uint128_t) a->val[1] *
    // b->val[1][1][0]; acc = (uint128_t) a->val[1] * b->val[1][1][0];
    // res->val[0] += acc;

    acc = ((uint128_t)a->val[1] * b->val[0][0][0]);
    res->val[1] = acc;
    acc = ((uint128_t)a->val[0] * b->val[0][1][1]);
    res->val[1] += acc;
    acc = ((uint128_t)a->val[2] * b->val[1][1][0]);
    res->val[0] += (((uint64_t)acc) & (((((uint64_t)1) << 46) - 1))) << 18;
    res->val[1] += (uint128_t)(acc >> 46);

    acc = ((uint128_t)a->val[2] * b->val[0][0][0]);
    res->val[2] = acc;
    acc = ((uint128_t)a->val[1] * b->val[0][1][1]);
    res->val[2] += acc;
}

static inline __attribute__((always_inline)) void
carry_round_KC_1305(field_elem_t *res, dfield_elem_t *a) {
    uint64_t c;

    // need to check how many rounds are necessary
    c = (uint64_t)(a->val[0] >> (64));
    a->val[1] += c;
    a->val[0] = (uint64_t)a->val[0];

    c = (uint64_t)(a->val[1] >> (64));
    res->val[1] = ((uint64_t)(a->val[1]));
    a->val[2] += c;
    c = (uint64_t)(a->val[2] >> (2));
    res->val[2] = ((uint64_t)(a->val[2])) & (((((uint64_t)1) << 2) - 1));

    a->val[0] += c * 5;
    res->val[0] = ((uint64_t)(a->val[0]));
    c = (a->val[0] >> 64);
    res->val[1] += c;
}

static inline __attribute__((always_inline)) void
carry_round_KC_1503(field_elem_t *res, dfield_elem_t *a) {
    uint64_t c;

    // need to check how many rounds are necessary
    c = (uint64_t)(a->val[0] >> (64));
    a->val[1] += c;
    a->val[0] = (uint64_t)a->val[0];

    c = (uint64_t)(a->val[1] >> (64));
    res->val[1] = ((uint64_t)(a->val[1]));
    a->val[2] += c;

    res->val[2] = ((uint64_t)(a->val[2])) & (((((uint64_t)1) << 22) - 1));

    a->val[2] >>= (22);
    // c = ((uint128_t) (a->val[2] >> (22))* 3);
    a->val[0] += ((uint128_t)a->val[2] * 3);
    res->val[0] = ((uint64_t)(a->val[0]));
    c = (a->val[0] >> 64);
    res->val[1] += c;
}

static inline __attribute__((always_inline)) void
carry_round_KC_1743(field_elem_t *res, dfield_elem_t *a) {
    uint128_t c;

    // need to check how many rounds are necessary
    c = (uint64_t)(a->val[0] >> (64));
    a->val[1] += c;
    a->val[0] = (uint64_t)a->val[0];

    c = (uint64_t)(a->val[1] >> (64));
    res->val[1] = ((uint64_t)(a->val[1]));
    a->val[2] += c;
    c = ((uint128_t)(a->val[2] >> (46)) * 3);
    res->val[2] = ((uint64_t)(a->val[2])) & (((((uint64_t)1) << 46) - 1));

    a->val[0] += ((uint128_t)c);
    res->val[0] = ((uint64_t)(a->val[0]));
    c = (a->val[0] >> 64);
    res->val[1] += c;
}

static inline __attribute__((always_inline)) int
reduce_KC_1305(field_elem_t *res, field_elem_t *a) {
    field_elem_t t;
    uint128_t td;
    uint64_t c;
    uint64_t mask;
    td = a->val[0] + 5;
    c = (uint64_t)(td >> (64));
    t.val[0] = ((uint64_t)(td));
    td = a->val[1] + c;
    c = (uint64_t)(td >> (64));
    t.val[1] = ((uint64_t)(td));
    t.val[2] = a->val[2] + c;
    t.val[2] += -(((uint64_t)1) << 2);
    mask = (uint64_t)(t.val[2] >> (63));
    mask += -1;
    t.val[0] = (t.val[0]) & (mask);
    t.val[1] = (t.val[1]) & (mask);
    t.val[2] = (t.val[2]) & (mask);
    mask = ~mask;
    res->val[0] = ((a->val[0]) & (mask)) | (t.val[0]);
    res->val[1] = ((a->val[1]) & (mask)) | (t.val[1]);
    res->val[2] = ((a->val[2]) & (mask)) | (t.val[2]);
    return 0;
}

static inline __attribute__((always_inline)) int
reduce_KC_1503(field_elem_t *res, field_elem_t *a) {
    field_elem_t t;
    uint128_t td;
    uint64_t c;
    uint64_t mask;
    td = a->val[0] + 3;
    c = (uint64_t)(td >> (64));
    t.val[0] = ((uint64_t)(td));
    td = a->val[1] + c;
    c = (uint64_t)(td >> (64));
    t.val[1] = ((uint64_t)(td));
    t.val[2] = a->val[2] + c;
    t.val[2] += -(((uint64_t)1) << 22);
    mask = (uint64_t)(t.val[2] >> (63));
    mask += -1;
    t.val[0] = (t.val[0]) & (mask);
    t.val[1] = (t.val[1]) & (mask);
    t.val[2] = (t.val[2]) & (mask);
    mask = ~mask;
    res->val[0] = ((a->val[0]) & (mask)) | (t.val[0]);
    res->val[1] = ((a->val[1]) & (mask)) | (t.val[1]);
    res->val[2] = ((a->val[2]) & (mask)) | (t.val[2]);
    return 0;
}

static inline __attribute__((always_inline)) int
reduce_KC_1743(field_elem_t *res, field_elem_t *a) {
    field_elem_t t;
    uint128_t td;
    uint64_t c;
    uint64_t mask;
    td = a->val[0] + 3;
    c = (uint64_t)(td >> (64));
    t.val[0] = ((uint64_t)(td));
    td = a->val[1] + c;
    c = (uint64_t)(td >> (64));
    t.val[1] = ((uint64_t)(td));
    t.val[2] = a->val[2] + c;
    t.val[2] += -(((uint64_t)1) << 46);
    mask = (uint64_t)(t.val[2] >> (63));
    mask += -1;
    t.val[0] = (t.val[0]) & (mask);
    t.val[1] = (t.val[1]) & (mask);
    t.val[2] = (t.val[2]) & (mask);
    mask = ~mask;
    res->val[0] = ((a->val[0]) & (mask)) | (t.val[0]);
    res->val[1] = ((a->val[1]) & (mask)) | (t.val[1]);
    res->val[2] = ((a->val[2]) & (mask)) | (t.val[2]);
    return 0;
}

#endif
