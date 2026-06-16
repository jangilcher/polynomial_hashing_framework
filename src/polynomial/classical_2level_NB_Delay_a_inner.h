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

#include "../field_arithmetic/field_arithmetic.h"
#include "../transform/transform.h"
#include "boost/preprocessor/arithmetic/add.hpp"
#include "boost/preprocessor/arithmetic/mul.hpp"
#include "boost/preprocessor/arithmetic/sub.hpp"
#include "boost/preprocessor/repetition/repeat.hpp"
#include "boost/preprocessor/repetition/repeat_from_to.hpp"
#include <stddef.h>
#include <string.h>

#ifdef ALWAYS_INLINE_INNER
#define INLINE static inline __attribute__((always_inline))
#else
#define INLINE static inline
#endif

#ifdef OUTER
#ifdef OUTER_PARAM0
#define NB_BLOCK_FLVL OUTER_PARAM0
#else
#define NB_BLOCK_FLVL 1
#endif
#else
#ifdef INNER_PARAM0
#define NB_BLOCK_FLVL INNER_PARAM0
#else
#define NB_BLOCK_FLVL 1
#endif
#endif

#define NB_BLOCK_FLVL_BLOCKSIZE NB_BLOCK_FLVL *BLOCKSIZE

#define COMPUTE_POWER_ITERATION(z, n, data)                                    \
    field_mul(data + n + 1, data + n, data);
#define UNPACK_AND_ENCODE(z, i, acc)                                           \
    unpack_and_encode_field_elem(acc + i, (baseint_t *)in);                    \
    in += BLOCKSIZE;                                                           \
    inlen -= BLOCKSIZE;
#define FIELD_MUL_NO_CARRY_ACC_A(z, n, data)                                   \
    field_mul_no_carry(acc_d + n, a + n, data - n);
#define FIELD_ADD_DBL_CHAIN(z, n, data) field_add_dbl(data, data, data + n);

// Classical 2 level with reduction after last addition
INLINE void classical_2level_NB_Delay_a_inner(field_elem_t *acc,
                                              const unsigned char *in,
                                              unsigned long long inlen,
                                              const unsigned char *key,
                                              int last) {

    memset(acc, 0, sizeof(field_elem_t));
    if (inlen == 0) {
        return;
    }
    unsigned int nbBlockFlvl = NB_BLOCK_FLVL;
    dfield_elem_t acc_d[nbBlockFlvl];
    memset(acc_d, 0, sizeof(acc_d));
    field_elem_t a[nbBlockFlvl]; // temporary field element representing the
                                 // block being processed
    field_elem_t k[nbBlockFlvl];
    // unsigned char buff[nbBlockFlvl][BUFFSIZE];
    unsigned char transkey[BUFFSIZE] = {0};
    unsigned long long nbBlockFlvl_blocksize = nbBlockFlvl * BLOCKSIZE;

    // Transform key from a byte array to one field elements
    transform_key(transkey, BUFFSIZE, key,
                  KEYSIZE); // transform key from bytes to a packed field
                            // elements of BLOCKSIZE bytes
    unpack_field_elem(k,
                      (baseint_t *)transkey); // transform field element(packed)
                                              // into limb representation

    // process msg of only 1 block
    if (inlen <= BLOCKSIZE) {
        // transform_msg(tag_packed, BUFFSIZE, in, inlen); // transform msg
        // block from bytes to field elements (packed)
        if (last) {
            unpack_and_encode_last_field_elem(acc, (baseint_t *)in, inlen);
        } else {
            unpack_and_encode_field_elem(acc, (baseint_t *)in);
        }
    } else if (inlen <= nbBlockFlvl_blocksize) { // process msg of nbBlockFlvl
                                                 // Blocks or less
#define IF_LESS_THAN_N_BLOCKS(z, i, data)                                      \
    if (inlen <= i * BLOCKSIZE) {                                              \
        BOOST_PP_REPEAT(BOOST_PP_SUB(i, 2), COMPUTE_POWER_ITERATION, k);       \
        BOOST_PP_REPEAT(BOOST_PP_SUB(i, 1), UNPACK_AND_ENCODE, a);             \
        if (last) {                                                            \
            unpack_and_encode_last_field_elem(a + nbBlockFlvl - i - 1,         \
                                              (baseint_t *)in, inlen);         \
        } else {                                                               \
            unpack_and_encode_field_elem(a + nbBlockFlvl - i - 1,              \
                                         (baseint_t *)in);                     \
        }                                                                      \
        field_mul_no_carry(acc_d + i - 2, a, k + i - 2);                       \
        BOOST_PP_REPEAT(BOOST_PP_SUB(i, 2), FIELD_MUL_NO_CARRY_ACC_A,          \
                        k + i - 2);                                            \
        BOOST_PP_REPEAT_FROM_TO(1, BOOST_PP_SUB(i, 2), FIELD_ADD_DBL_CHAIN,    \
                                acc_d);                                        \
        field_add_mix(acc_d, acc_d, a + i - 1);                                \
        carry_round(acc, acc_d);                                               \
    } else
        BOOST_PP_REPEAT_FROM_TO(2, BOOST_PP_ADD(NB_BLOCK_FLVL, 1),
                                IF_LESS_THAN_N_BLOCKS, 0) {}
        reduce(acc, acc);
    } else { // process msg of more than nbBlockFlvl Blocks
        // compute key powers for parallel horner

        BOOST_PP_REPEAT(BOOST_PP_SUB(NB_BLOCK_FLVL, 1), COMPUTE_POWER_ITERATION,
                        k);

        // initialize accumulator
        BOOST_PP_REPEAT(NB_BLOCK_FLVL, UNPACK_AND_ENCODE, a);

        BOOST_PP_REPEAT(BOOST_PP_SUB(NB_BLOCK_FLVL, 1),
                        FIELD_MUL_NO_CARRY_ACC_A, k + NB_BLOCK_FLVL - 2);
        BOOST_PP_REPEAT_FROM_TO(1, BOOST_PP_SUB(NB_BLOCK_FLVL, 1),
                                FIELD_ADD_DBL_CHAIN, acc_d);

        field_add_mix(acc_d, acc_d, a + nbBlockFlvl - 1);
        carry_round(acc, acc_d);

        while (inlen > NB_BLOCK_FLVL_BLOCKSIZE) {
            BOOST_PP_REPEAT(NB_BLOCK_FLVL, UNPACK_AND_ENCODE, a);
            field_mul_no_carry(acc_d + NB_BLOCK_FLVL - 1, acc,
                               k + nbBlockFlvl - 1);
            BOOST_PP_REPEAT(BOOST_PP_SUB(NB_BLOCK_FLVL, 1),
                            FIELD_MUL_NO_CARRY_ACC_A, k + NB_BLOCK_FLVL - 2);
            BOOST_PP_REPEAT_FROM_TO(1, NB_BLOCK_FLVL, FIELD_ADD_DBL_CHAIN,
                                    acc_d);
            field_add_mix(acc_d, acc_d, a + nbBlockFlvl - 1);
            carry_round(acc, acc_d);
        }
        // process remaining msg of length less or equal to nbBlockFlvl Blocks
#define IF_MORE_THAN_N_REMAINING_BLOCKS(z, i, data)                            \
    if (inlen > BOOST_PP_SUB(BOOST_PP_SUB(NB_BLOCK_FLVL, i), 1) * BLOCKSIZE) { \
        BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_SUB(NB_BLOCK_FLVL, i), 1),       \
                        UNPACK_AND_ENCODE, a);                                 \
        if (last) {                                                            \
            unpack_and_encode_last_field_elem(a + nbBlockFlvl - i - 1,         \
                                              (baseint_t *)in, inlen);         \
        } else {                                                               \
            unpack_and_encode_field_elem(a + nbBlockFlvl - i - 1,              \
                                         (baseint_t *)in);                     \
        }                                                                      \
        field_mul_no_carry(acc_d + BOOST_PP_SUB(NB_BLOCK_FLVL, i) - 1, acc,    \
                           k + nbBlockFlvl - i - 1);                           \
        BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_SUB(NB_BLOCK_FLVL, i), 1),       \
                        FIELD_MUL_NO_CARRY_ACC_A,                              \
                        k + BOOST_PP_SUB(NB_BLOCK_FLVL, i) - 2);               \
        BOOST_PP_REPEAT_FROM_TO(1, BOOST_PP_SUB(NB_BLOCK_FLVL, i),             \
                                FIELD_ADD_DBL_CHAIN, acc_d);                   \
        field_add_mix(acc_d, acc_d, a + nbBlockFlvl - i - 1);                  \
        carry_round(acc, acc_d);                                               \
    } else
        BOOST_PP_REPEAT(NB_BLOCK_FLVL, IF_MORE_THAN_N_REMAINING_BLOCKS, 0){};
        reduce(acc, acc);
    }
}
