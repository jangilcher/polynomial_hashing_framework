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

#if defined(INNERPOLY_H) && defined(INNERPOLY)
#include INNERPOLY_H
#endif
#ifdef OUTER_PARAM0
#define NB_BLOCK_FLVL OUTER_PARAM0
#else
#define NB_BLOCK_FLVL 1
#endif

#define NB_BLOCK_FLVL_BLOCKSIZE NB_BLOCK_FLVL *BLOCKSIZE

#define COMPUTE_POWER_ITERATION(z, n, data)                                    \
    field_mul(NOT_PRECOMPUTED(data) + n + 1, NOT_PRECOMPUTED(data) + n,        \
              NOT_PRECOMPUTED(data));
#define UNPACK_AND_ENCODE(z, i, acc)                                           \
    unpack_and_encode_field_elem(acc + i, (baseint_t *)in);                    \
    in += BLOCKSIZE;                                                           \
    inlen -= BLOCKSIZE;
#define FIELD_MUL_NO_CARRY_ACC_A(z, n, data)                                   \
    FIELD_MUL_PC_NO_CARRY(acc_d + n, a + n, data - n);
#define FIELD_ADD_DBL_CHAIN(z, n, data) field_add_dbl(data, data, data + n);

// Classical 2 level with reduction after last addition
void classical_2level_NB_UPK_Delay_a(unsigned char *out,
                                     const unsigned char *in,
                                     unsigned long long inlen,
                                     const unsigned char *key) {
    if (inlen == 0) {
        memset(out, 0, OUTPUTSIZE);
        return;
    }
    field_elem_t acc;
    dfield_elem_t acc_d[NB_BLOCK_FLVL];
    field_elem_t a[NB_BLOCK_FLVL];
    DECLARE_PC_ELEM_ARRAY(k, NB_BLOCK_FLVL);
    unsigned char transkey[BUFFSIZE] = {0};
    unsigned char tag_packed[BUFFSIZE] = {0};

    // Transform key from a byte array to one field elements
    transform_key(transkey, BUFFSIZE, key, KEYSIZE);
    UNPACK_PC_FIELD_ELEM(k, transkey, 0);
    INIT_PC_KEY(k, NOT_PRECOMPUTED(k));

    if (inlen <= BLOCKSIZE) {
        // transform msg block from bytes to field elements (packed)
        unpack_and_encode_last_field_elem(&acc, (baseint_t *)in, inlen);
        pack_field_elem((baseint_t *)tag_packed, &acc);
    } else if (inlen <= NB_BLOCK_FLVL_BLOCKSIZE) {
#define PRECOMPUTE_KEY_POWER(z, i, data)                                       \
    FIELD_MUL_PC_NO_CARRY(acc_d, NOT_PRECOMPUTED(k) + i, k);                   \
    carry_round(NOT_PRECOMPUTED(k) + BOOST_PP_ADD(i, 1), acc_d);               \
    INIT_PC_KEY(k + BOOST_PP_ADD(i, 1),                                        \
                NOT_PRECOMPUTED(k) + BOOST_PP_ADD(i, 1));
#define IF_LESS_THAN_N_BLOCKS(z, i, data)                                      \
    if (inlen <= i * BLOCKSIZE) {                                              \
        BOOST_PP_REPEAT(BOOST_PP_SUB(i, 2), PRECOMPUTE_KEY_POWER, 0)           \
        BOOST_PP_REPEAT(BOOST_PP_SUB(i, 1), UNPACK_AND_ENCODE, a);             \
        unpack_and_encode_last_field_elem(a + BOOST_PP_SUB(i, 1),              \
                                          (baseint_t *)in, inlen);             \
        BOOST_PP_REPEAT(BOOST_PP_SUB(i, 1), FIELD_MUL_NO_CARRY_ACC_A,          \
                        k + BOOST_PP_SUB(i, 2));                               \
        BOOST_PP_REPEAT_FROM_TO(1, BOOST_PP_SUB(i, 1), FIELD_ADD_DBL_CHAIN,    \
                                acc_d);                                        \
        field_add_mix(acc_d, acc_d, a + BOOST_PP_SUB(i, 1));                   \
        carry_round(&acc, acc_d);                                              \
    } else
        BOOST_PP_REPEAT_FROM_TO(2, BOOST_PP_ADD(NB_BLOCK_FLVL, 1),
                                IF_LESS_THAN_N_BLOCKS, 0) {}
        _carry_round(&acc, &acc);
        reduce(&acc, &acc);
        pack_field_elem((baseint_t *)tag_packed, &acc);
    } else { // process msg of more than nbBlockFlvl Blocks
        // compute key powers for parallel horner

        for (int j = 0; j < BOOST_PP_SUB(NB_BLOCK_FLVL, 1); ++j) {
            FIELD_MUL_PC_NO_CARRY(acc_d, NOT_PRECOMPUTED(k) + j, k);
            carry_round(NOT_PRECOMPUTED(k) + j + 1, acc_d);
            INIT_PC_KEY(k + j + 1, NOT_PRECOMPUTED(k) + j + 1);
        }

        // initialize accumulator
        BOOST_PP_REPEAT(NB_BLOCK_FLVL, UNPACK_AND_ENCODE, a);

        BOOST_PP_REPEAT(BOOST_PP_SUB(NB_BLOCK_FLVL, 1),
                        FIELD_MUL_NO_CARRY_ACC_A, k + NB_BLOCK_FLVL - 2);
        BOOST_PP_REPEAT_FROM_TO(1, BOOST_PP_SUB(NB_BLOCK_FLVL, 1),
                                FIELD_ADD_DBL_CHAIN, acc_d);

        field_add_mix(acc_d, acc_d, a + BOOST_PP_SUB(NB_BLOCK_FLVL, 1));
        carry_round(&acc, acc_d);

        while (inlen > NB_BLOCK_FLVL_BLOCKSIZE) {
            BOOST_PP_REPEAT(NB_BLOCK_FLVL, UNPACK_AND_ENCODE, a);
            FIELD_MUL_PC_NO_CARRY(acc_d + BOOST_PP_SUB(NB_BLOCK_FLVL, 1), &acc,
                                  k + BOOST_PP_SUB(NB_BLOCK_FLVL, 1));
            BOOST_PP_REPEAT(BOOST_PP_SUB(NB_BLOCK_FLVL, 1),
                            FIELD_MUL_NO_CARRY_ACC_A, k + NB_BLOCK_FLVL - 2);
            BOOST_PP_REPEAT_FROM_TO(1, NB_BLOCK_FLVL, FIELD_ADD_DBL_CHAIN,
                                    acc_d);
            field_add_mix(acc_d, acc_d, a + BOOST_PP_SUB(NB_BLOCK_FLVL, 1));
            carry_round(&acc, acc_d);
        }
        // process remaining msg of length less or equal to nbBlockFlvl Blocks
#define IF_MORE_THAN_N_REMAINING_BLOCKS(z, i, data)                            \
    if (inlen > BOOST_PP_SUB(BOOST_PP_SUB(NB_BLOCK_FLVL, i), 1) * BLOCKSIZE) { \
        BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_SUB(NB_BLOCK_FLVL, i), 1),       \
                        UNPACK_AND_ENCODE, a);                                 \
        unpack_and_encode_last_field_elem(                                     \
            a + BOOST_PP_SUB(BOOST_PP_SUB(NB_BLOCK_FLVL, i), 1),               \
            (baseint_t *)in, inlen);                                           \
        FIELD_MUL_PC_NO_CARRY(                                                 \
            acc_d + BOOST_PP_SUB(BOOST_PP_SUB(NB_BLOCK_FLVL, i), 1), &acc,     \
            k + NB_BLOCK_FLVL - i - 1);                                        \
        BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_SUB(NB_BLOCK_FLVL, i), 1),       \
                        FIELD_MUL_NO_CARRY_ACC_A,                              \
                        k + BOOST_PP_SUB(BOOST_PP_SUB(NB_BLOCK_FLVL, i), 2));  \
        BOOST_PP_REPEAT_FROM_TO(1, BOOST_PP_SUB(NB_BLOCK_FLVL, i),             \
                                FIELD_ADD_DBL_CHAIN, acc_d);                   \
        field_add_mix(acc_d, acc_d,                                            \
                      a + BOOST_PP_SUB(BOOST_PP_SUB(NB_BLOCK_FLVL, i), 1));    \
        carry_round(&acc, acc_d);                                              \
    } else
        BOOST_PP_REPEAT(NB_BLOCK_FLVL, IF_MORE_THAN_N_REMAINING_BLOCKS, 0){};

        _carry_round(&acc, &acc);
        reduce(&acc, &acc);
        pack_field_elem((baseint_t *)tag_packed,
                        &acc); // transform limb representation of field element
                               // (unpacked) into packed field element
    }
    transform_field_elem(
        out, OUTPUTSIZE, tag_packed,
        BUFFSIZE); // transform from field element to byte missing?
}
