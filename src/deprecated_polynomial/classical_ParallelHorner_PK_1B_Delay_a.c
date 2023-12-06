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

#ifdef OUTER_PARAM0
#define NB_BRANCH OUTER_PARAM0
#else
#define NB_BRANCH 1
#endif

#define NB_BRANCH_BLOCKSIZE NB_BRANCH *BLOCKSIZE

#if defined(INNERPOLY_H) && defined(INNERPOLY)
#include INNERPOLY_H
#endif

// reduction after addition only
void classical_ParallelHorner_PK_1B_Delay_a(unsigned char *out,
                                            const unsigned char *in,
                                            unsigned long long inlen,
                                            const unsigned char *key) {
    if (inlen == 0) {
        memset(out, 0, OUTPUTSIZE);
        return;
    }
    field_elem_t acc[NB_BRANCH] = {0};    // accumulator for horner algorithm
    dfield_elem_t acc_d[NB_BRANCH] = {0}; // accumulator for horner algorithm
    field_elem_t a[NB_BRANCH] = {
        0}; // temporary field element representing the block being processed
    field_elem_t k[NB_BRANCH] = {0};
    field_elem_precomputed_t k_p[NB_BRANCH];
    unsigned char transkey[BUFFSIZE] = {0};
    unsigned char tag_packed[BUFFSIZE] = {0};
    // unsigned long long NB_BRANCH_BLOCKSIZE = NB_BRANCH*BLOCKSIZE;

    // Transform key from a byte array to one field elements
    transform_key(transkey, BUFFSIZE, key, KEYSIZE);
    unpack_field_elem(k, (baseint_t *)transkey);
    precompute_factor(k_p, k);

#define UNPACK_AND_ENCODE(z, i, acc)                                           \
    unpack_and_encode_field_elem(acc + i, (baseint_t *)in);                    \
    in += BLOCKSIZE;                                                           \
    inlen -= BLOCKSIZE;

#define FIELD_MUL_NO_CARRY_ACC_SCALAR(z, i, s)                                 \
    field_mul_precomputed_no_carry(acc_d + i, acc + i, s);

#define FIELD_MUL_NO_CARRY_ACC_VEC(z, i, s)                                    \
    field_mul_precomputed_no_carry(acc_d + i, acc + i, s + i);

#define FIELD_MUL_NO_CARRY_ACC_VEC_REV(z, i, s)                                \
    field_mul_precomputed_no_carry(acc_d + i, acc + i, s - i);

#define FIELD_ADD_MIX_ACC(z, i, s) field_add_mix(acc_d + i, acc_d + i, s + i);

#define CARRY_ROUND_ACC(z, i, _) carry_round(acc + i, acc_d + i);

    // process msg of only 1 block
    if (inlen <= BLOCKSIZE) {
        // transform_msg(tag_packed, BUFFSIZE, in, inlen); // transform msg
        // block from bytes to field elements (packed)
        unpack_and_encode_last_field_elem(
            acc, (baseint_t *)in,
            inlen); // Not optimized, unpack then pack is
                    // redundant, only need encode/transform
        pack_field_elem((baseint_t *)tag_packed,
                        acc); // Not optimized, unpack then pack is
                              // redundant, only need encode/transform
    } else {
        // compute key powers for parallel horner
        for (int j = 0; j < NB_BRANCH - 1; ++j) {
            field_mul_precomputed_no_carry(acc_d, k + j, k_p);
            carry_round(k + j + 1, acc_d);
            precompute_factor(k_p + j + 1, k + j + 1);
        }
#define IF_LESS_THAN_N_BLOCKS(z, i, data)                                      \
    if (inlen <= i * BLOCKSIZE) {                                              \
        BOOST_PP_REPEAT(BOOST_PP_SUB(i, 1), UNPACK_AND_ENCODE, acc)            \
        unpack_and_encode_last_field_elem(acc + BOOST_PP_SUB(i, 1),            \
                                          (baseint_t *)in, inlen);             \
                                                                               \
        BOOST_PP_REPEAT(BOOST_PP_SUB(i, 1), FIELD_MUL_NO_CARRY_ACC_VEC_REV,    \
                        k_p + BOOST_PP_SUB(i, 2))                              \
        carry_round(acc, acc_d);                                               \
        for (int j = 1; j < i - 1; ++j) {                                      \
            field_add_mix(acc_d, acc_d + j, acc);                              \
            carry_round(acc, acc_d);                                           \
        }                                                                      \
        field_add_reduce(acc, acc, acc + BOOST_PP_SUB(i, 1));                  \
        pack_field_elem((baseint_t *)tag_packed, acc);                         \
    } else
        // end IF_LESS_THAN_N_BLOCKS
        BOOST_PP_REPEAT_FROM_TO(2, BOOST_PP_ADD(NB_BRANCH, 1),
                                IF_LESS_THAN_N_BLOCKS, 0)
        // ELSE BLOCK!
        { // process msg of more than NB_BRANCH Blocks
            // initialize accumulators
            BOOST_PP_REPEAT(NB_BRANCH, UNPACK_AND_ENCODE, acc)

            while (inlen > NB_BRANCH_BLOCKSIZE) {
                BOOST_PP_REPEAT(NB_BRANCH, UNPACK_AND_ENCODE, a)

                BOOST_PP_REPEAT(NB_BRANCH, FIELD_MUL_NO_CARRY_ACC_SCALAR,
                                (k_p + NB_BRANCH - 1))
                BOOST_PP_REPEAT(NB_BRANCH, FIELD_ADD_MIX_ACC, a)
                BOOST_PP_REPEAT(NB_BRANCH, CARRY_ROUND_ACC, 0)
            }
            // process msg of nbBlocks blocks with last one possibly
            // incomplete
#define REDUCE_ACCUMULATORS(z, i, acc)                                         \
    field_add_mix(acc##_d, acc##_d + i, acc);                                  \
    carry_round(acc, acc##_d);
#define IF_MORE_THAN_N_REMAINING_BLOCKS(z, i, data)                            \
    if (inlen > BOOST_PP_SUB(NB_BRANCH, i) * BLOCKSIZE) {                      \
        BOOST_PP_REPEAT(BOOST_PP_SUB(NB_BRANCH, i), UNPACK_AND_ENCODE, a)      \
        unpack_and_encode_last_field_elem(a + BOOST_PP_SUB(NB_BRANCH, i),      \
                                          (baseint_t *)in, inlen);             \
                                                                               \
        BOOST_PP_REPEAT(BOOST_PP_SUB(NB_BRANCH, i),                            \
                        FIELD_MUL_NO_CARRY_ACC_SCALAR,                         \
                        k_p + BOOST_PP_SUB(NB_BRANCH, 1))                      \
        BOOST_PP_REPEAT_FROM_TO(BOOST_PP_SUB(NB_BRANCH, i), NB_BRANCH,         \
                                FIELD_MUL_NO_CARRY_ACC_VEC_REV,                \
                                k_p +                                          \
                                    BOOST_PP_ADD(BOOST_PP_SUB(NB_BRANCH, i),   \
                                                 BOOST_PP_SUB(NB_BRANCH, 1)))  \
        BOOST_PP_REPEAT(BOOST_PP_ADD(BOOST_PP_SUB(NB_BRANCH, i), 1),           \
                        FIELD_ADD_MIX_ACC, a)                                  \
        BOOST_PP_REPEAT(BOOST_PP_SUB(NB_BRANCH, i), CARRY_ROUND_ACC, 0)        \
                                                                               \
        BOOST_PP_REPEAT(BOOST_PP_SUB(NB_BRANCH, i),                            \
                        FIELD_MUL_NO_CARRY_ACC_VEC_REV,                        \
                        k_p + BOOST_PP_SUB(BOOST_PP_SUB(NB_BRANCH, 1), i))     \
        carry_round(acc, acc_d);                                               \
        BOOST_PP_REPEAT_FROM_TO(1, NB_BRANCH, REDUCE_ACCUMULATORS, acc)        \
    } else
            // end IF_MORE_THAN_N_REMAINING_BLOCKS
            BOOST_PP_REPEAT_FROM_TO(1, BOOST_PP_ADD(NB_BRANCH, 1),
                                    IF_MORE_THAN_N_REMAINING_BLOCKS, 0) {}
            reduce(acc, acc);
            pack_field_elem(
                (baseint_t *)tag_packed,
                acc); // transform limb representation of field element
                      // (unpacked) into packed field element
        }
    }
    transform_field_elem(
        out, OUTPUTSIZE, tag_packed,
        BUFFSIZE); // transform from field element to byte missing?
}
