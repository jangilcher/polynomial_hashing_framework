// MIT License
//
// Copyright (c) 2025 Jan Gilcher, Jérôme Govinden
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

#include "../field_arithmetic/field_arithmetic.h"
#include "../transform/transform.h"
#include "boost/preprocessor/arithmetic/add.hpp"
#include "boost/preprocessor/arithmetic/div.hpp"
#include "boost/preprocessor/arithmetic/mod.hpp"
#include "boost/preprocessor/arithmetic/mul.hpp"
#include "boost/preprocessor/arithmetic/sub.hpp"
#include "boost/preprocessor/repetition/repeat.hpp"
#include "boost/preprocessor/repetition/repeat_from_to.hpp"
#include <stddef.h>
#include <string.h>


static inline __attribute__((always_inline)) int unpack_and_encode_newkey(
    field_elem_t* res,
    const uint64_t* a)
{
//    res->val[0] = a[0];
//    res->val[1] = a[1];
    
    res->val[0] = (((a[0]))) & (((((uint64_t) 1) << 58) - 1));
    res->val[1] = (((uint64_t*) &(((uint8_t*) a)[7]))[0] >> 2) & 0x3fffffffffffff;
    
    return 0;
}

static inline __attribute__((always_inline)) int unpack_and_encode_new_field_elem(
    field_elem_t* res,
    const uint64_t* a)
{
//    res->val[0] = a[0];
//    res->val[1] = a[1];
    
    res->val[0] = (((a[0]))) & (((((uint64_t) 1) << 58) - 1));
    res->val[1] = (((uint64_t*) &(((uint8_t*) a)[7]))[0] >> 2) & 0x3fffffffffffff;
    
    return 0;
}

static inline __attribute__((always_inline)) int field_mul_new_no_carry(
    dfield_elem_t* res,
    const field_elem_t* a,
    const field_elem_t* b)
{
    //uint128_t acc;
    uint64_t t;
    res->val[0] = ((uint128_t) a->val[0] * b->val[0]);
    //t = ((uint64_t) b->val[1] * (3));
    t = ((uint64_t) (b->val[1] << 1) + b->val[1]);
    res->val[0] += ((uint128_t) a->val[1] * t);

    res->val[1] = ((uint128_t) a->val[0] * b->val[1]);
    res->val[1] += ((uint128_t) a->val[1] * b->val[0]);

    return 0;
}


#ifdef ALWAYS_INLINE_INNER
#define INLINE static inline __attribute__((always_inline))
#else
#define INLINE static inline
#endif

// PARAM0 is number of multiplications processed in one loop (before delay
// carry)
// PARAM1 indicates if the polynomial uses fully delayed limb reduction
// (i.e. only one reduction at the end)
// PARAM2 indicates carry before multiplication
// PARAM3 indicates the usage of the same accumulator inside the inner loop (PARAM3=1) or the use of multiple parallel ones (PARAM3=0)
#ifdef OUTER
#ifdef OUTER_PARAM0
#define NB_MULT_DELAY OUTER_PARAM0
#else
#define NB_MULT_DELAY 1
#endif
#ifdef OUTER_PARAM1
#define FULL_DELAY OUTER_PARAM1
#else
#define FULL_DELAY 0
#endif
#ifdef OUTER_PARAM2
#if OUTER_PARAM2
#define CARRY_ADD
#endif
#endif
#ifdef OUTER_PARAM3
#define USE_SAME_ACC OUTER_PARAM3
#else
#define USE_SAME_ACC 0
#endif
#else
#ifdef INNER_PARAM0
#define NB_MULT_DELAY INNER_PARAM0
#else
#define NB_MULT_DELAY 1
#endif
#ifdef INNER_PARAM1
#define FULL_DELAY INNER_PARAM1
#else
#define FULL_DELAY 0
#endif
#ifdef INNER_PARAM2
#if INNER_PARAM2
#define CARRY_ADD
#endif
#endif
#ifdef INNER_PARAM3
#define USE_SAME_ACC INNER_PARAM3
#else
#define USE_SAME_ACC 0
#endif
#endif

#define NB_BLOCK_DELAY BOOST_PP_MUL(2, NB_MULT_DELAY)
#define NB_BLOCK_DELAY_BLOCKSIZE (NB_BLOCK_DELAY * BLOCKSIZE)

#if FULL_DELAY
#define DECLARE_FULL_DELAY_ACC dfield_elem_t f_acc = {0};
#define ADD_TO_ACC(dest) field_add_dbl(dest, &f_acc, acc_d);
#define INTERMEDIATE_CARRY
#define ADD_FINAL_BLOCK                                                        \
    if (inlen > 0) {                                                           \
        UNPACK_AND_ENCODE_LAST_FIELD_ELEM(a, (baseint_t *)in, inlen);          \
        field_add_mix(acc_d, &f_acc, a);                                       \
        carry_round(&acc, acc_d);                                              \
    } else {                                                                   \
        carry_round(&acc, &f_acc);                                             \
    };
#else
#define DECLARE_FULL_DELAY_ACC
#define ADD_TO_ACC(dest) field_add_mix(acc_d, acc_d, &acc);
#define INTERMEDIATE_CARRY carry_round(&acc, acc_d);
#define ADD_FINAL_BLOCK                                                        \
    if (inlen > 0) {                                                           \
        UNPACK_AND_ENCODE_LAST_FIELD_ELEM(a, (baseint_t *)in, inlen);          \
        field_add(&acc, &acc, a);                                              \
        _carry_round(&acc, &acc);                                              \
    }
#endif

#define KEYSTREAM key
#define KEY_SETUP

#ifdef CARRY_ADD
#define FIELD_ADD(z, i, data)                                                  \
    field_add(data + i, data + i, k + i);                                      \
    _carry_round(data + i, data + i);
#else
#define FIELD_ADD(z, i, data) field_add(data + i, data + i, k + i);
#endif
#define FIELD_MUL_NO_CARRY(z, i, data)                                         \
    field_mul_new_no_carry(acc_d + i, data + BOOST_PP_MUL(2, i),                   \
                       data + BOOST_PP_MUL(2, i) + 1);

#define FIELD_ADD_DBL_CHAIN(z, i, data) field_add_dbl(data, data, data + i);

#if defined(OUTER) || defined(NO_INNER_CACHE)
#define UNPACK_AND_ENCODE_MSG_AND_KEY(z, i, data)                              \
    unpack_and_encode_new_field_elem(data + i, (baseint_t *)in);                   \
    unpack_and_encode_newkey(k + i, (baseint_t *)KEYSTREAM);                      \
    in += BLOCKSIZE;                                                           \
    KEYSTREAM += KEYSIZE;                                                      \
    inlen -= BLOCKSIZE;

#define INCREASE_KEY_INDEX

#define UNPACK_AND_ENCODE_KEY(res, k) unpack_and_encode_newkey((res), (k))

#define FINALIZE_DELAY_BLOCK

#else
#define NB_KEYS NB_SUPERBLOCKS
#define UNPACK_AND_ENCODE_MSG_AND_KEY(z, i, data)                              \
    unpack_and_encode_new_field_elem(data + i, (baseint_t *)in);                   \
    in += BLOCKSIZE;                                                           \
    inlen -= BLOCKSIZE;

#define INCREASE_KEY_INDEX k += 2;

#define UNPACK_AND_ENCODE_KEY(res, k)

#define NB_KEYS_BATCHES                                                        \
    (NB_KEYS / NB_BLOCK_DELAY) + !!(NB_KEYS % NB_BLOCK_DELAY)
typedef struct NMH_NB_Delay_state {
    field_elem_t key[NB_KEYS_BATCHES][NB_BLOCK_DELAY];
} NMH_NB_Delay_inner_state_t;

#define FINALIZE_DELAY_BLOCK k += NB_BLOCK_DELAY;

#define INNER_STATE_T NMH_NB_Delay_inner_state_t
#define INNER_STATE_INIT NMH_NB_Delay_inner_init_state

INLINE void NMH_NB_Delay_inner_init_state(NMH_NB_Delay_inner_state_t *state,
                                          const unsigned char *key) {
#if NB_KEYS == NB_SUPERKEYS
    const unsigned char *maxkey = key + SUPERKEYSIZE;
    for (int j = 0; j < NB_KEYS_BATCHES && (key < maxkey); j++)
        for (int i = 0; i < NB_BLOCK_DELAY && (key < maxkey); i++) {
            unpack_and_encode_newkey(&state->key[j][i], (baseint_t *)key);
            key += KEYSIZE;
        }

#else
    field_elem_t *st_key = (field_elem_t *)state->key;
    unpack_and_encode_newkey((field_elem_t *)st_key, (baseint_t *)key);

#ifdef __GNUC__
#ifdef __clang__
#pragma unroll 65534
#else
#pragma GCC unroll 65534
#endif
#endif
    for (int j = 1; j < NB_KEYS; j++) {
        field_mul(&st_key[j], &st_key[j - 1], &st_key[0]);
    }
#endif
}
#if SUPERKEYSIZE
#define INNER_STATE_ZERO                                                       \
    { 0 }
#else
#define INNER_STATE_ZERO                                                       \
    {}
#endif
#endif

#if defined(OUTER) || defined(NO_INNER_CACHE)
INLINE void NMH_tweak_NB_Delay1163_inner(field_elem_t *out, const unsigned char *in,
                               unsigned long long inlen,
                               const unsigned char *key, int last)
#else
INLINE void NMH_tweak_NB_Delay1163_inner(field_elem_t *out, const unsigned char *in,
                               unsigned long long inlen,
                               NMH_NB_Delay_inner_state_t *state, int last)
#endif
{

    if (inlen == 0) {
        memset(out, 0, sizeof(field_elem_t));
        return;
    }

// field_elem_t acc = {0};
#define acc *out
    acc = (field_elem_t){0};
    DECLARE_FULL_DELAY_ACC;
    field_elem_t a[NB_BLOCK_DELAY];
    dfield_elem_t acc_d[NB_MULT_DELAY] = {0};
#if defined(OUTER) || defined(NO_INNER_CACHE)
    field_elem_t keys[NB_BLOCK_DELAY];
    field_elem_t *k = keys;
#else
    field_elem_t *k = (field_elem_t *)state->key;
#endif
    KEY_SETUP;

    // Process msg of more than NB_BLOCK_DELAY Blocks
    while (inlen > NB_BLOCK_DELAY_BLOCKSIZE) {
#if USE_SAME_ACC
        BOOST_PP_REPEAT(2, UNPACK_AND_ENCODE_MSG_AND_KEY, a);
        BOOST_PP_REPEAT(2, FIELD_ADD, a);
        BOOST_PP_REPEAT(1, FIELD_MUL_NO_CARRY, a);
        INCREASE_KEY_INDEX;
        for(int j=1; j< NB_MULT_DELAY; ++j) {
            BOOST_PP_REPEAT(2, UNPACK_AND_ENCODE_MSG_AND_KEY, a);
            BOOST_PP_REPEAT(2, FIELD_ADD, a);
            field_mul_new_no_carry(acc_d+1, a, a + 1);
            field_add_dbl(acc_d, acc_d, acc_d + 1);
            INCREASE_KEY_INDEX;
        }
#else
        BOOST_PP_REPEAT(NB_BLOCK_DELAY, UNPACK_AND_ENCODE_MSG_AND_KEY, a);
        BOOST_PP_REPEAT(NB_BLOCK_DELAY, FIELD_ADD, a);
        BOOST_PP_REPEAT(NB_MULT_DELAY, FIELD_MUL_NO_CARRY, a);
        BOOST_PP_REPEAT_FROM_TO(1, NB_MULT_DELAY, FIELD_ADD_DBL_CHAIN, acc_d);
        FINALIZE_DELAY_BLOCK;
#endif
        ADD_TO_ACC(&f_acc);
        INTERMEDIATE_CARRY;
    }
    // process remaining msg of length less or equal to NB_BLOCK_DELAY Blocks
#define IF_MORE_THAN_N_REMAINING_BLOCKS(z, i, data)                            \
    if (inlen >                                                                \
        BOOST_PP_SUB(BOOST_PP_SUB(NB_BLOCK_DELAY, i), 1) * BLOCKSIZE) {        \
        BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_SUB(NB_BLOCK_DELAY, i), 1),      \
                        UNPACK_AND_ENCODE_MSG_AND_KEY, a);                     \
        UNPACK_AND_ENCODE_LAST_FIELD_ELEM(                                     \
            a + BOOST_PP_SUB(BOOST_PP_SUB(NB_BLOCK_DELAY, i), 1),              \
            (baseint_t *)in, inlen);                                           \
        UNPACK_AND_ENCODE_KEY(                                                 \
            k + BOOST_PP_SUB(BOOST_PP_SUB(NB_BLOCK_DELAY, i), 1),              \
            (baseint_t *)KEYSTREAM);                                           \
        BOOST_PP_REPEAT(                                                       \
            BOOST_PP_MUL(BOOST_PP_DIV(BOOST_PP_SUB(NB_BLOCK_DELAY, i), 2), 2), \
            FIELD_ADD, a);                                                     \
        BOOST_PP_REPEAT(BOOST_PP_DIV(BOOST_PP_SUB(NB_BLOCK_DELAY, i), 2),      \
                        FIELD_MUL_NO_CARRY, a);                                \
        BOOST_PP_REPEAT_FROM_TO(                                               \
            1, BOOST_PP_DIV(BOOST_PP_SUB(NB_BLOCK_DELAY, i), 2),               \
            FIELD_ADD_DBL_CHAIN, acc_d);                                       \
        ADD_TO_ACC(acc_d);                                                     \
        if (BOOST_PP_MOD(BOOST_PP_SUB(NB_BLOCK_DELAY, i), 2)) {                \
            field_add_mix(                                                     \
                acc_d, acc_d,                                                  \
                a + BOOST_PP_SUB(BOOST_PP_SUB(NB_BLOCK_DELAY, i), 1));         \
        }                                                                      \
        carry_round(&acc, acc_d);                                              \
    } else
    BOOST_PP_REPEAT(BOOST_PP_SUB(NB_BLOCK_DELAY, 1),
                    IF_MORE_THAN_N_REMAINING_BLOCKS, 0)
    ADD_FINAL_BLOCK;

#undef acc
    //_carry_round(&acc, &acc);
    // reduce(out, &acc);
}
