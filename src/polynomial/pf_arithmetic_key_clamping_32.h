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

#ifndef field_arithmetic_key_clamping_32_H_
#define field_arithmetic_key_clamping_32_H_

#include "../field_arithmetic/field_arithmetic.h"

#define LAST_MSG_BLOCKSIZE (8 * BLOCKSIZE - 128)
#define LAST_FIELDELEM_BLOCKSIZE (PI - 128)

#ifndef OUTER_PARAM0
#define OUTER_PARAM0 32
#endif
#define WORDSIZE OUTER_PARAM0

#ifndef OUTER_PARAM1
#define OUTER_PARAM1 5
#endif

#ifndef OUTER_PARAM2
#define OUTER_PARAM2 0
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
// upper bit key clamping with 4 limbs key

static inline __attribute__((always_inline)) void
unpack_field_elem_KC(field_elem_t *res, uint32_t *a) {
    res->val[0] = a[0];
    res->val[1] = a[1];
    res->val[2] = a[2];
    res->val[3] = a[3];
    res->val[4] = a[4] & ((((uint32_t)1) << LAST_MSG_BLOCKSIZE) - 1);
    res->val[4] |= (uint32_t)0x0 << LAST_MSG_BLOCKSIZE;
}

static inline __attribute__((always_inline)) void
unpack_and_encode_field_elem_KC(field_elem_t *res, uint32_t *a) {
    res->val[0] = a[0];
    res->val[1] = a[1];
    res->val[2] = a[2];
    res->val[3] = a[3];
    res->val[4] = a[4] & ((((uint32_t)1) << LAST_MSG_BLOCKSIZE) - 1);
    res->val[4] |= (uint32_t)OUTER_PARAM2 << LAST_MSG_BLOCKSIZE;
}

static inline __attribute__((always_inline)) void
unpack_and_encode_last_field_elem_KC(field_elem_t *res, uint32_t *a,
                                     size_t size) {
    uint32_t tmp[BUFFSIZE] = {0};
    memcpy(tmp, a, size);
    ((uint8_t *)tmp)[size] = (uint8_t)OUTER_PARAM2;
    res->val[0] = tmp[0];
    res->val[1] = tmp[1];
    res->val[2] = tmp[2];
    res->val[3] = tmp[3];
    res->val[4] = tmp[4];
}

static inline __attribute__((always_inline)) void
pack_field_elem_KC(uint32_t *res, field_elem_t *a) {
    res[0] = (a->val[0]);
    res[1] = (a->val[1]);
    res[2] = (a->val[2]);
    res[3] = (a->val[3]);
    res[4] = (a->val[4]);
}

#if OUTER_PARAM3 == 0
// if OUTER_PARAM3 = 0 then uses a mixed of low and upper bit key clamping as
// poly1305
static inline __attribute__((always_inline)) void
precompute_factor_KC(field_elem_precomputed_t *res, field_elem_t *b) {
    res->val[0][0][0] = b->val[0];
    res->val[0][1][1] = b->val[1];
    res->val[0][2][2] = b->val[2];
    res->val[0][3][3] = b->val[3];

    res->val[1][1][1] =
        ((uint32_t)(b->val[1] >> LAST_FIELDELEM_BLOCKSIZE) * DELTA);
    res->val[1][2][2] =
        ((uint32_t)(b->val[2] >> LAST_FIELDELEM_BLOCKSIZE) * DELTA);
    res->val[1][3][3] =
        ((uint32_t)(b->val[3] >> LAST_FIELDELEM_BLOCKSIZE) * DELTA);
}
#elif OUTER_PARAM3 <= 2
// if OUTER_PARAM3 = 1 then uses a mixed of low and upper bit key clamping as
// poly1305 (technique described in blog post) if OUTER_PARAM3 = 2 then uses
// full low and upper bit key clamping
static inline __attribute__((always_inline)) void
precompute_factor_KC(field_elem_precomputed_t *res, field_elem_t *b) {
    res->val[0][0][0] = b->val[0];
    res->val[0][1][1] = b->val[1];
    res->val[0][2][2] = b->val[2];
    res->val[0][3][3] = b->val[3];

    res->val[1][0][0] =
        ((uint32_t)(b->val[0] >> LAST_FIELDELEM_BLOCKSIZE) * DELTA);
    res->val[1][1][1] =
        ((uint32_t)(b->val[1] >> LAST_FIELDELEM_BLOCKSIZE) * DELTA);
    res->val[1][2][2] =
        ((uint32_t)(b->val[2] >> LAST_FIELDELEM_BLOCKSIZE) * DELTA);
    res->val[1][3][3] =
        ((uint32_t)(b->val[3] >> LAST_FIELDELEM_BLOCKSIZE) * DELTA);
}
#elif OUTER_PARAM3 == 3
// if OUTER_PARAM3 = 3 then uses upper bit key clamping with 4 limbs key
static inline __attribute__((always_inline)) void
precompute_factor_KC(field_elem_precomputed_t *res, field_elem_t *b) {
    res->val[0][0][0] = b->val[0];
    res->val[0][1][1] = b->val[1];
    res->val[0][2][2] = b->val[2];
    res->val[0][3][3] = b->val[3];

    res->val[1][1][1] = ((uint32_t)b->val[1] * DELTA);
    res->val[1][2][2] = ((uint32_t)b->val[2] * DELTA);
    res->val[1][3][3] = ((uint32_t)b->val[3] * DELTA);
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
//    res->val[0] = ((uint64_t) a->val[0] * b->val[0][0][0]);
//    res->val[0] += ((uint64_t) a->val[1] * b->val[1][3][3]);
//    res->val[0] += ((uint64_t) a->val[2] * b->val[1][2][2]);
//    res->val[0] += ((uint64_t) a->val[3] * b->val[1][1][1]);
//
//    res->val[1] = ((uint64_t) a->val[1] * b->val[0][0][0]);
//    res->val[1] += ((uint64_t) a->val[0] * b->val[0][1][1]);
//    res->val[1] += ((uint64_t) a->val[2] * b->val[1][3][3]);
//    res->val[1] += ((uint64_t) a->val[3] * b->val[1][2][2]);
//    res->val[1] += ((uint32_t) a->val[4] * b->val[1][1][1]);
//
//    res->val[2] = ((uint64_t) a->val[2] * b->val[0][0][0]);
//    res->val[2] += ((uint64_t) a->val[1] * b->val[0][1][1]);
//    res->val[2] += ((uint64_t) a->val[0] * b->val[0][2][2]);
//    res->val[2] += ((uint64_t) a->val[3] * b->val[1][3][3]);
//    res->val[2] += ((uint32_t) a->val[4] * b->val[1][2][2]);
//
//    res->val[3] = ((uint64_t) a->val[3] * b->val[0][0][0]);
//    res->val[3] += ((uint64_t) a->val[2] * b->val[0][1][1]);
//    res->val[3] += ((uint64_t) a->val[1] * b->val[0][2][2]);
//    res->val[3] += ((uint64_t) a->val[0] * b->val[0][3][3]);
//    res->val[3] += ((uint32_t) a->val[4] * b->val[1][3][3]);
//
//    res->val[4] = ((uint32_t) a->val[4] * b->val[0][0][0]);
//}
static inline __attribute__((always_inline)) void
field_mul_precomputed_no_carry_KC(dfield_elem_t *res, field_elem_t *a,
                                  field_elem_precomputed_t *b) {
    res->val[0] = ((uint64_t)a->val[0] * b->val[0][0][0]);
    res->val[0] += ((uint64_t)a->val[1] * b->val[1][3][3]);
    res->val[0] += ((uint64_t)a->val[2] * b->val[1][2][2]);
    res->val[0] += ((uint64_t)a->val[3] * b->val[1][1][1]);

    res->val[1] = ((uint64_t)a->val[1] * b->val[0][0][0]);
    res->val[1] += ((uint64_t)a->val[0] * b->val[0][1][1]);
    res->val[1] += ((uint64_t)a->val[2] * b->val[1][3][3]);
    res->val[1] += ((uint64_t)a->val[3] * b->val[1][2][2]);
    res->val[1] += ((uint64_t)a->val[4] * b->val[1][1][1]);

    res->val[2] = ((uint64_t)a->val[2] * b->val[0][0][0]);
    res->val[2] += ((uint64_t)a->val[1] * b->val[0][1][1]);
    res->val[2] += ((uint64_t)a->val[0] * b->val[0][2][2]);
    res->val[2] += ((uint64_t)a->val[3] * b->val[1][3][3]);
    res->val[2] += ((uint64_t)a->val[4] * b->val[1][2][2]);

    res->val[3] = ((uint64_t)a->val[3] * b->val[0][0][0]);
    res->val[3] += ((uint64_t)a->val[2] * b->val[0][1][1]);
    res->val[3] += ((uint64_t)a->val[1] * b->val[0][2][2]);
    res->val[3] += ((uint64_t)a->val[0] * b->val[0][3][3]);
    res->val[3] += ((uint64_t)a->val[4] * b->val[1][3][3]);

    res->val[4] = ((uint64_t)a->val[4] * b->val[0][0][0]);
}
#elif OUTER_PARAM3 == 1
// if OUTER_PARAM3 = 1 then uses a mixed of low and upper bit key clamping as
// poly1305 (technique described in blog post)
static inline __attribute__((always_inline)) void
field_mul_precomputed_no_carry_KC(dfield_elem_t *res, field_elem_t *a,
                                  field_elem_precomputed_t *b) {
    res->val[0] = ((uint64_t)a->val[0] * b->val[0][0][0]);
    res->val[0] += ((uint64_t)a->val[1] * b->val[1][3][3]);
    res->val[0] += ((uint64_t)a->val[2] * b->val[1][2][2]);
    res->val[0] += ((uint64_t)a->val[3] * b->val[1][1][1]);
    res->val[0] += ((uint32_t)a->val[4] * b->val[1][0][0]);

    res->val[1] = ((uint64_t)a->val[1] * b->val[0][0][0]);
    res->val[1] += ((uint64_t)a->val[0] * b->val[0][1][1]);
    res->val[1] += ((uint64_t)a->val[2] * b->val[1][3][3]);
    res->val[1] += ((uint64_t)a->val[3] * b->val[1][2][2]);
    res->val[1] += ((uint32_t)a->val[4] * b->val[1][1][1]);

    res->val[2] = ((uint64_t)a->val[2] * b->val[0][0][0]);
    res->val[2] += ((uint64_t)a->val[1] * b->val[0][1][1]);
    res->val[2] += ((uint64_t)a->val[0] * b->val[0][2][2]);
    res->val[2] += ((uint64_t)a->val[3] * b->val[1][3][3]);
    res->val[2] += ((uint32_t)a->val[4] * b->val[1][2][2]);

    res->val[3] = ((uint64_t)a->val[3] * b->val[0][0][0]);
    res->val[3] += ((uint64_t)a->val[2] * b->val[0][1][1]);
    res->val[3] += ((uint64_t)a->val[1] * b->val[0][2][2]);
    res->val[3] += ((uint64_t)a->val[0] * b->val[0][3][3]);
    res->val[3] += ((uint32_t)a->val[4] * b->val[1][3][3]);

    res->val[4] = ((uint32_t)a->val[4] *
                   (b->val[0][0][0] &
                    (((((uint32_t)1) << LAST_FIELDELEM_BLOCKSIZE) - 1))));
}
#elif OUTER_PARAM3 == 2
// if OUTER_PARAM3 = 2 then uses full low and upper bit key clamping
// static inline __attribute__((always_inline)) void
// field_mul_precomputed_no_carry_KC(
//    dfield_elem_t* res,
//    field_elem_t* a,
//    field_elem_precomputed_t* b)
//{
//    res->val[0] = ((uint64_t) a->val[0] * b->val[0][0][0]);
//    res->val[0] += ((uint64_t) a->val[1] * b->val[1][3][3]);
//    res->val[0] += ((uint64_t) a->val[2] * b->val[1][2][2]);
//    res->val[0] += ((uint64_t) a->val[3] * b->val[1][1][1]);
//    res->val[0] += ((uint32_t) a->val[4] * b->val[1][0][0]);
//
//    res->val[1] = ((uint64_t) a->val[1] * b->val[0][0][0]);
//    res->val[1] += ((uint64_t) a->val[0] * b->val[0][1][1]);
//    res->val[1] += ((uint64_t) a->val[2] * b->val[1][3][3]);
//    res->val[1] += ((uint64_t) a->val[3] * b->val[1][2][2]);
//    res->val[1] += ((uint32_t) a->val[4] * b->val[1][1][1]);
//
//    res->val[2] = ((uint64_t) a->val[2] * b->val[0][0][0]);
//    res->val[2] += ((uint64_t) a->val[1] * b->val[0][1][1]);
//    res->val[2] += ((uint64_t) a->val[0] * b->val[0][2][2]);
//    res->val[2] += ((uint64_t) a->val[3] * b->val[1][3][3]);
//    res->val[2] += ((uint32_t) a->val[4] * b->val[1][2][2]);
//
//    res->val[3] = ((uint64_t) a->val[3] * b->val[0][0][0]);
//    res->val[3] += ((uint64_t) a->val[2] * b->val[0][1][1]);
//    res->val[3] += ((uint64_t) a->val[1] * b->val[0][2][2]);
//    res->val[3] += ((uint64_t) a->val[0] * b->val[0][3][3]);
//    res->val[3] += ((uint32_t) a->val[4] * b->val[1][3][3]);
//
//    res->val[4] = 0;
//}
static inline __attribute__((always_inline)) void
field_mul_precomputed_no_carry_KC(dfield_elem_t *res, field_elem_t *a,
                                  field_elem_precomputed_t *b) {
    res->val[0] = ((uint64_t)a->val[0] * b->val[0][0][0]);
    res->val[0] += ((uint64_t)a->val[1] * b->val[1][3][3]);
    res->val[0] += ((uint64_t)a->val[2] * b->val[1][2][2]);
    res->val[0] += ((uint64_t)a->val[3] * b->val[1][1][1]);
    res->val[0] += ((uint64_t)a->val[4] * b->val[1][0][0]);

    res->val[1] = ((uint64_t)a->val[1] * b->val[0][0][0]);
    res->val[1] += ((uint64_t)a->val[0] * b->val[0][1][1]);
    res->val[1] += ((uint64_t)a->val[2] * b->val[1][3][3]);
    res->val[1] += ((uint64_t)a->val[3] * b->val[1][2][2]);
    res->val[1] += ((uint64_t)a->val[4] * b->val[1][1][1]);

    res->val[2] = ((uint64_t)a->val[2] * b->val[0][0][0]);
    res->val[2] += ((uint64_t)a->val[1] * b->val[0][1][1]);
    res->val[2] += ((uint64_t)a->val[0] * b->val[0][2][2]);
    res->val[2] += ((uint64_t)a->val[3] * b->val[1][3][3]);
    res->val[2] += ((uint64_t)a->val[4] * b->val[1][2][2]);

    res->val[3] = ((uint64_t)a->val[3] * b->val[0][0][0]);
    res->val[3] += ((uint64_t)a->val[2] * b->val[0][1][1]);
    res->val[3] += ((uint64_t)a->val[1] * b->val[0][2][2]);
    res->val[3] += ((uint64_t)a->val[0] * b->val[0][3][3]);
    res->val[3] += ((uint64_t)a->val[4] * b->val[1][3][3]);

    res->val[4] = 0;
}
#elif OUTER_PARAM3 == 3
// if OUTER_PARAM3 = 3 then uses upper bit key clamping with 4 limbs key
static inline __attribute__((always_inline)) void
field_mul_precomputed_no_carry_KC(dfield_elem_t *res, field_elem_t *a,
                                  field_elem_precomputed_t *b) {
    uint64_t acc;

    res->val[0] = ((uint64_t)a->val[0] * b->val[0][0][0]);

    res->val[1] = ((uint64_t)a->val[1] * b->val[0][0][0]);
    res->val[1] += ((uint64_t)a->val[0] * b->val[0][1][1]);

    res->val[2] = ((uint64_t)a->val[2] * b->val[0][0][0]);
    res->val[2] += ((uint64_t)a->val[1] * b->val[0][1][1]);
    res->val[2] += ((uint64_t)a->val[0] * b->val[0][2][2]);

    res->val[3] = ((uint64_t)a->val[3] * b->val[0][0][0]);
    res->val[3] += ((uint64_t)a->val[2] * b->val[0][1][1]);
    res->val[3] += ((uint64_t)a->val[1] * b->val[0][2][2]);
    res->val[3] += ((uint64_t)a->val[0] * b->val[0][3][3]);

    res->val[4] = ((uint64_t)a->val[4] * b->val[0][0][0]);
    res->val[4] += ((uint64_t)a->val[3] * b->val[0][1][1]);
    res->val[4] += ((uint64_t)a->val[2] * b->val[0][2][2]);
    res->val[4] += ((uint64_t)a->val[1] * b->val[0][3][3]);

    acc = ((uint64_t)a->val[4] * b->val[1][1][1]);
    acc += ((uint64_t)a->val[3] * b->val[1][2][2]);
    acc += ((uint64_t)a->val[2] * b->val[1][3][3]);
    res->val[0] +=
        (((uint32_t)acc) & (((((uint32_t)1) << LAST_FIELDELEM_BLOCKSIZE) - 1)))
        << (WORDSIZE - LAST_FIELDELEM_BLOCKSIZE);
    res->val[1] += (uint64_t)((uint64_t)acc >> LAST_FIELDELEM_BLOCKSIZE);

    acc = ((uint64_t)a->val[4] * b->val[1][2][2]);
    acc += ((uint64_t)a->val[3] * b->val[1][3][3]);
    res->val[1] +=
        (((uint32_t)acc) & (((((uint32_t)1) << LAST_FIELDELEM_BLOCKSIZE) - 1)))
        << (WORDSIZE - LAST_FIELDELEM_BLOCKSIZE);
    res->val[2] += (uint64_t)((uint64_t)acc >> LAST_FIELDELEM_BLOCKSIZE);

    acc = ((uint64_t)a->val[4] * b->val[1][3][3]);
    res->val[2] +=
        (((uint32_t)acc) & (((((uint32_t)1) << LAST_FIELDELEM_BLOCKSIZE) - 1)))
        << (WORDSIZE - LAST_FIELDELEM_BLOCKSIZE);
    res->val[3] += (uint64_t)((uint64_t)acc >> LAST_FIELDELEM_BLOCKSIZE);
}
#endif

#if (OUTER_PARAM3 == 0) || (OUTER_PARAM3 == 3)
// if OUTER_PARAM3 = 0 then uses a mixed of low and upper bit key clamping as
// poly1305 if OUTER_PARAM3 = 3 then uses upper bit key clamping with 4 limbs
// key
static inline __attribute__((always_inline)) void
carry_round_KC(field_elem_t *res, dfield_elem_t *a) {
    // need to check how many rounds are necessary
    // a->val[4] += (uint32_t) (a->val[3] >> (32));
    // a->val[3] = ((uint32_t) (a->val[3])) ;

    a->val[0] += ((uint64_t)(a->val[4] >> LAST_FIELDELEM_BLOCKSIZE) *
                  DELTA); // 32 vs 64 for up
    res->val[4] = ((uint32_t)(a->val[4])) &
                  (((((uint32_t)1) << LAST_FIELDELEM_BLOCKSIZE) - 1));

    a->val[1] += (uint32_t)(a->val[0] >> (32));
    res->val[0] = ((uint32_t)(a->val[0]));

    a->val[2] += (uint32_t)(a->val[1] >> (32));
    res->val[1] = ((uint32_t)(a->val[1]));

    a->val[3] += (uint32_t)(a->val[2] >> (32));
    res->val[2] = ((uint32_t)(a->val[2]));

    res->val[4] += (uint32_t)(a->val[3] >> (32));
    res->val[3] = ((uint32_t)(a->val[3]));
}
#elif OUTER_PARAM3 == 1
// if OUTER_PARAM3 = 1 then uses a mixed of low and upper bit key clamping as
// poly1305 (technique described in blog post)
static inline __attribute__((always_inline)) void
carry_round_KC(field_elem_t *res, dfield_elem_t *a) {
    // need to check how many rounds are necessary
    a->val[4] += (uint32_t)(a->val[3] >> (32));
    a->val[3] = ((uint32_t)(a->val[3]));

    a->val[0] += ((uint32_t)(a->val[4] >> LAST_FIELDELEM_BLOCKSIZE) *
                  DELTA); // 32 vs 64 for up
    res->val[4] = ((uint32_t)(a->val[4])) &
                  (((((uint32_t)1) << LAST_FIELDELEM_BLOCKSIZE) - 1));

    a->val[1] += (uint32_t)(a->val[0] >> (32));
    res->val[0] = ((uint32_t)(a->val[0]));

    a->val[2] += (uint32_t)(a->val[1] >> (32));
    res->val[1] = ((uint32_t)(a->val[1]));

    a->val[3] += (uint32_t)(a->val[2] >> (32));
    res->val[2] = ((uint32_t)(a->val[2]));

    res->val[4] += (uint32_t)(a->val[3] >> (32));
    res->val[3] = ((uint32_t)(a->val[3]));
}
#elif OUTER_PARAM3 == 2
// if OUTER_PARAM3 = 2 then uses full low and upper bit key clamping
static inline __attribute__((always_inline)) void
carry_round_KC(field_elem_t *res, dfield_elem_t *a) {
    // a->val[0] += ((uint64_t) (a->val[4] >> LAST_FIELDELEM_BLOCKSIZE)* DELTA);
    res->val[4] = ((uint32_t)(a->val[4]));

    a->val[1] += (uint32_t)(a->val[0] >> (32));
    res->val[0] = ((uint32_t)(a->val[0]));

    a->val[2] += (uint32_t)(a->val[1] >> (32));
    res->val[1] = ((uint32_t)(a->val[1]));

    a->val[3] += (uint32_t)(a->val[2] >> (32));
    res->val[2] = ((uint32_t)(a->val[2]));

    res->val[4] += (uint32_t)(a->val[3] >> (32));
    res->val[3] = ((uint32_t)(a->val[3]));
}
#endif

#if (OUTER_PARAM3 == 0) || (OUTER_PARAM3 == 2) || (OUTER_PARAM3 == 3)
// if OUTER_PARAM3 = 0 then uses a mixed of low and upper bit key clamping as
// poly1305 if OUTER_PARAM3 = 2 then uses full low and upper bit key clamping if
// OUTER_PARAM3 = 3 then uses upper bit key clamping with 4 limbs key
static inline __attribute__((always_inline)) void
carry_round_KC_final(field_elem_t *res, dfield_elem_t *a) {
    // need to check how many rounds are necessary
    a->val[4] += (uint32_t)(a->val[3] >> (32));
    a->val[3] = ((uint32_t)(a->val[3]));

    a->val[0] += ((uint64_t)(a->val[4] >> LAST_FIELDELEM_BLOCKSIZE) *
                  DELTA); // 32 vs 64 for up
    res->val[4] = ((uint32_t)(a->val[4])) &
                  (((((uint32_t)1) << LAST_FIELDELEM_BLOCKSIZE) - 1));

    a->val[1] += (uint32_t)(a->val[0] >> (32));
    res->val[0] = ((uint32_t)(a->val[0]));

    a->val[2] += (uint32_t)(a->val[1] >> (32));
    res->val[1] = ((uint32_t)(a->val[1]));

    a->val[3] += (uint32_t)(a->val[2] >> (32));
    res->val[2] = ((uint32_t)(a->val[2]));

    res->val[4] += (uint32_t)(a->val[3] >> (32));
    res->val[3] = ((uint32_t)(a->val[3]));
}
#elif OUTER_PARAM3 == 1
// if OUTER_PARAM3 = 1 then uses a mixed of low and upper bit key clamping as
// poly1305 (technique described in blog post)
// similar as carry_round_KC
static inline __attribute__((always_inline)) void
carry_round_KC_final(field_elem_t *res, dfield_elem_t *a) {
    // need to check how many rounds are necessary
    a->val[4] += (uint32_t)(a->val[3] >> (32));
    a->val[3] = ((uint32_t)(a->val[3]));

    a->val[0] += ((uint32_t)(a->val[4] >> LAST_FIELDELEM_BLOCKSIZE) *
                  DELTA); // 32 vs 64 for up
    res->val[4] = ((uint32_t)(a->val[4])) &
                  (((((uint32_t)1) << LAST_FIELDELEM_BLOCKSIZE) - 1));

    a->val[1] += (uint32_t)(a->val[0] >> (32));
    res->val[0] = ((uint32_t)(a->val[0]));

    a->val[2] += (uint32_t)(a->val[1] >> (32));
    res->val[1] = ((uint32_t)(a->val[1]));

    a->val[3] += (uint32_t)(a->val[2] >> (32));
    res->val[2] = ((uint32_t)(a->val[2]));

    res->val[4] += (uint32_t)(a->val[3] >> (32));
    res->val[3] = ((uint32_t)(a->val[3]));
}
#endif

static inline __attribute__((always_inline)) int reduce_KC(field_elem_t *res,
                                                           field_elem_t *a) {
    field_elem_t t;
    uint64_t td;
    uint32_t c;
    uint32_t mask;
    td = a->val[0] + DELTA;
    c = (uint32_t)(td >> (32));
    t.val[0] = ((uint32_t)(td));
    td = a->val[1] + c;
    c = (uint32_t)(td >> (32));
    t.val[1] = ((uint32_t)(td));
    td = a->val[2] + c;
    c = (uint32_t)(td >> (32));
    t.val[2] = ((uint32_t)(td));
    td = a->val[3] + c;
    c = (uint32_t)(td >> (32));
    t.val[3] = ((uint32_t)(td));
    t.val[4] = a->val[4] + c;
    t.val[4] += -(((uint32_t)1) << LAST_FIELDELEM_BLOCKSIZE);
    mask = (uint32_t)(t.val[4] >> (31));
    mask += -1;
    t.val[0] = (t.val[0]) & (mask);
    t.val[1] = (t.val[1]) & (mask);
    t.val[2] = (t.val[2]) & (mask);
    t.val[3] = (t.val[3]) & (mask);
    t.val[4] = (t.val[4]) & (mask);
    mask = ~mask;
    res->val[0] = ((a->val[0]) & (mask)) | (t.val[0]);
    res->val[1] = ((a->val[1]) & (mask)) | (t.val[1]);
    res->val[2] = ((a->val[2]) & (mask)) | (t.val[2]);
    res->val[3] = ((a->val[3]) & (mask)) | (t.val[3]);
    res->val[4] = ((a->val[4]) & (mask)) | (t.val[4]);
    return 0;
}

static inline __attribute__((always_inline)) void
unpack_field_elem_KC_1305_32(field_elem_t *res, uint32_t *a) {
    res->val[0] = a[0];
    res->val[1] = a[1];
    res->val[2] = a[2];
    res->val[3] = a[3];
    res->val[4] = 0;
}

static inline __attribute__((always_inline)) void
unpack_and_encode_field_elem_KC_32(field_elem_t *res, uint32_t *a) {
    res->val[0] = a[0];
    res->val[1] = a[1];
    res->val[2] = a[2];
    res->val[3] = a[3];
    res->val[4] = 1;
}

static inline __attribute__((always_inline)) void
unpack_and_encode_last_field_elem_KC_32(field_elem_t *res, uint32_t *a,
                                        size_t size) {
    uint32_t tmp[BUFFSIZE] = {0};
    memcpy(tmp, a, size);
    if (size < 16) {
        ((uint8_t *)tmp)[size] = 0x1;
    }
    res->val[0] = tmp[0];
    res->val[1] = tmp[1];
    res->val[2] = tmp[2];
    res->val[3] = tmp[3];
    res->val[4] = (size == 16);
    //  res->val[4] = 0;
    //    if (size == 16) {
    //        res->val[4] = 1;
    //    }
}

static inline __attribute__((always_inline)) void
pack_field_elem_KC_32(uint32_t *res, field_elem_t *a) {
    res[0] = (a->val[0]);
    res[1] = (a->val[1]);
    res[2] = (a->val[2]);
    res[3] = (a->val[3]);
    res[4] = (a->val[4]);
}

static inline __attribute__((always_inline)) int
precompute_factor_KC_1305_32(field_elem_precomputed_t *res, field_elem_t *b) {
    res->val[0][0][0] = b->val[0];
    res->val[0][1][1] = b->val[1];
    res->val[0][2][2] = b->val[2];
    res->val[0][3][3] = b->val[3];

    res->val[1][1][1] = (b->val[1] >> 2) * 5;
    res->val[1][2][2] = (b->val[2] >> 2) * 5;
    res->val[1][3][3] = (b->val[3] >> 2) * 5;
    return 0;
}

static inline __attribute__((always_inline)) int
field_mul_precomputed_no_carry_KC_1305_32(dfield_elem_t *res, field_elem_t *a,
                                          field_elem_precomputed_t *b) {
    uint64_t acc;
    acc = ((uint64_t)a->val[0] * b->val[0][0][0]);
    res->val[0] = acc;
    acc = ((uint64_t)a->val[1] * b->val[1][3][3]);
    res->val[0] += acc;
    acc = ((uint64_t)a->val[2] * b->val[1][2][2]);
    res->val[0] += acc;
    acc = ((uint64_t)a->val[3] * b->val[1][1][1]);
    res->val[0] += acc;

    acc = ((uint64_t)a->val[1] * b->val[0][0][0]);
    res->val[1] = acc;
    acc = ((uint64_t)a->val[0] * b->val[0][1][1]);
    res->val[1] += acc;
    acc = ((uint64_t)a->val[2] * b->val[1][3][3]);
    res->val[1] += acc;
    acc = ((uint64_t)a->val[3] * b->val[1][2][2]);
    res->val[1] += acc;
    acc = ((uint64_t)a->val[4] * b->val[1][1][1]);
    res->val[1] += acc;

    acc = ((uint64_t)a->val[2] * b->val[0][0][0]);
    res->val[2] = acc;
    acc = ((uint64_t)a->val[1] * b->val[0][1][1]);
    res->val[2] += acc;
    acc = ((uint64_t)a->val[0] * b->val[0][2][2]);
    res->val[2] += acc;
    acc = ((uint64_t)a->val[3] * b->val[1][3][3]);
    res->val[2] += acc;
    acc = ((uint64_t)a->val[4] * b->val[1][2][2]);
    res->val[2] += acc;

    acc = ((uint64_t)a->val[3] * b->val[0][0][0]);
    res->val[3] = acc;
    acc = ((uint64_t)a->val[2] * b->val[0][1][1]);
    res->val[3] += acc;
    acc = ((uint64_t)a->val[1] * b->val[0][2][2]);
    res->val[3] += acc;
    acc = ((uint64_t)a->val[0] * b->val[0][3][3]);
    res->val[3] += acc;
    acc = ((uint64_t)a->val[4] * b->val[1][3][3]);
    res->val[3] += acc;

    acc = (a->val[4] * b->val[0][0][0]);
    res->val[4] = acc;

    return 0;
}

static inline __attribute__((always_inline)) int
carry_round_KC_1305_32(field_elem_t *res, dfield_elem_t *a) {
    uint32_t c;
    // need to check how many rounds are necessary
    c = (uint32_t)(a->val[0] >> (32));
    a->val[0] = (uint32_t)a->val[0];
    a->val[1] += c;

    c = (uint32_t)(a->val[1] >> (32));
    res->val[1] = ((uint32_t)(a->val[1]));
    a->val[2] += c;

    c = (uint32_t)(a->val[2] >> (32));
    res->val[2] = ((uint32_t)(a->val[2]));
    a->val[3] += c;

    c = (uint32_t)(a->val[3] >> (32));
    res->val[3] = ((uint32_t)(a->val[3]));
    a->val[4] += c;

    c = (uint32_t)(a->val[4] >> (2));
    res->val[4] = ((uint32_t)(a->val[4])) & (((((uint32_t)1) << 2) - 1));
    a->val[0] += c * 5;

    c = (a->val[0] >> 32);
    res->val[0] = (a->val[0]);
    res->val[1] += c;
    return 0;
}

// this needs to be changed
static inline __attribute__((always_inline)) int
reduce_KC_1305_32(field_elem_t *res, field_elem_t *a) {
    field_elem_t t;
    uint64_t c;
    uint64_t mask;
    t.val[0] = a->val[0] + 5;
    c = (uint32_t)(t.val[0] >> (26));
    t.val[0] = ((uint32_t)(t.val[0])) & (((((uint32_t)1) << 26) - 1));
    t.val[1] = a->val[1] + c;
    c = (uint32_t)(t.val[1] >> (26));
    t.val[1] = ((uint32_t)(t.val[1])) & (((((uint32_t)1) << 26) - 1));
    t.val[2] = a->val[2] + c;
    c = (uint32_t)(t.val[2] >> (26));
    t.val[2] = ((uint32_t)(t.val[2])) & (((((uint32_t)1) << 26) - 1));
    t.val[3] = a->val[3] + c;
    c = (uint32_t)(t.val[3] >> (26));
    t.val[3] = ((uint32_t)(t.val[3])) & (((((uint32_t)1) << 26) - 1));
    t.val[4] = a->val[4] + c;
    t.val[4] += -(((uint32_t)1) << 26);
    mask = (uint32_t)(t.val[4] >> (31));
    mask += -1;
    t.val[0] = (t.val[0]) & (mask);
    t.val[1] = (t.val[1]) & (mask);
    t.val[2] = (t.val[2]) & (mask);
    t.val[3] = (t.val[3]) & (mask);
    t.val[4] = (t.val[4]) & (mask);
    mask = ~mask;
    res->val[0] = ((a->val[0]) & (mask)) | (t.val[0]);
    res->val[1] = ((a->val[1]) & (mask)) | (t.val[1]);
    res->val[2] = ((a->val[2]) & (mask)) | (t.val[2]);
    res->val[3] = ((a->val[3]) & (mask)) | (t.val[3]);
    res->val[4] = ((a->val[4]) & (mask)) | (t.val[4]);
    return 0;
}

#endif
