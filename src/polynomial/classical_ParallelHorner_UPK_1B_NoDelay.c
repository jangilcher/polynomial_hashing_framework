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

#define UNPACK_AND_ENCODE(z, i, acc)                                           \
    unpack_and_encode_field_elem(acc + i, (baseint_t *)in);                    \
    in += BLOCKSIZE;                                                           \
    inlen -= BLOCKSIZE;

#define FIELD_MUL_ACC_SCALAR(z, i, s) FIELD_MUL_PC_REDUCE(acc + i, acc + i, s);

#define FIELD_MUL_ACC_VEC(z, i, s) FIELD_MUL_PC_REDUCE(acc + i, acc + i, s + i);

#define FIELD_MUL_ACC_VEC_REV(z, i, s)                                         \
    FIELD_MUL_PC_REDUCE(acc + i, acc + i, s - i);

#define FIELD_ADD_ACC(z, i, s) field_add_reduce(acc + i, acc + i, s + i);

#define REDUCE_ACCUMULATORS(z, i, acc) field_add_reduce(acc, acc + i, acc);

#if defined(INNERPOLY_H) && defined(INNERPOLY)
#include INNERPOLY_H
#endif

// reduction after addition only
void classical_ParallelHorner_UPK_1B_NoDelay(unsigned char *out,
                                             const unsigned char *in,
                                             unsigned long long inlen,
                                             const unsigned char *key) {
    if (inlen == 0) {
        memset(out, 0, OUTPUTSIZE);
        return;
    }
    field_elem_t acc[NB_BRANCH] = {0};
    field_elem_t a[NB_BRANCH] = {0};
    DECLARE_PC_ELEM_ARRAY(k, NB_BRANCH);
    unsigned char transkey[BUFFSIZE] = {0};
    unsigned char tag_packed[BUFFSIZE] = {0};

    // Transform key from a byte array to one field elements
    transform_key(transkey, BUFFSIZE, key, KEYSIZE);

    UNPACK_PC_FIELD_ELEM(k, transkey, 0);
    INIT_PC_KEY(k, NOT_PRECOMPUTED(k));

    // process msg of only 1 block
    if (inlen <= BLOCKSIZE) {
        // transform msg  block from bytes to field elements (packed)
        // Not optimized, unpack then pack is redundant, only need
        // encode/transform
        unpack_and_encode_last_field_elem(acc, (baseint_t *)in, inlen);
        pack_field_elem((baseint_t *)tag_packed, acc);
    } else {
        // compute key powers for parallel horner
        for (int j = 0; j < NB_BRANCH - 1; ++j) {
            FIELD_MUL_PC(NOT_PRECOMPUTED(k) + j + 1, NOT_PRECOMPUTED(k) + j, k);
            INIT_PC_KEY(k + j + 1, NOT_PRECOMPUTED(k) + j + 1);
        }
#define IF_LESS_THAN_N_BLOCKS(z, i, data)                                      \
    if (inlen <= i * BLOCKSIZE) {                                              \
        BOOST_PP_REPEAT(BOOST_PP_SUB(i, 1), UNPACK_AND_ENCODE, acc)            \
        unpack_and_encode_last_field_elem(acc + BOOST_PP_SUB(i, 1),            \
                                          (baseint_t *)in, inlen);             \
                                                                               \
        BOOST_PP_REPEAT(BOOST_PP_SUB(i, 1), FIELD_MUL_ACC_VEC_REV,             \
                        k + BOOST_PP_SUB(i, 2))                                \
        for (int j = 1; j < i; ++j) {                                          \
            field_add_reduce(acc, acc + j, acc);                               \
        }                                                                      \
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
                BOOST_PP_REPEAT(NB_BRANCH, FIELD_MUL_ACC_SCALAR,
                                (k + NB_BRANCH - 1))
                BOOST_PP_REPEAT(NB_BRANCH, FIELD_ADD_ACC, a)
            }
            // process msg of nbBlocks blocks with last one possibly
            // incomplete
#define IF_MORE_THAN_N_REMAINING_BLOCKS(z, i, data)                            \
    if (inlen > BOOST_PP_SUB(NB_BRANCH, i) * BLOCKSIZE) {                      \
        BOOST_PP_REPEAT(BOOST_PP_SUB(NB_BRANCH, i), UNPACK_AND_ENCODE, a)      \
        unpack_and_encode_last_field_elem(a + BOOST_PP_SUB(NB_BRANCH, i),      \
                                          (baseint_t *)in, inlen);             \
                                                                               \
        BOOST_PP_REPEAT(BOOST_PP_SUB(NB_BRANCH, i), FIELD_MUL_ACC_SCALAR,      \
                        k + BOOST_PP_SUB(NB_BRANCH, 1))                        \
        BOOST_PP_REPEAT_FROM_TO(BOOST_PP_SUB(NB_BRANCH, i), NB_BRANCH,         \
                                FIELD_MUL_ACC_VEC_REV,                         \
                                k + BOOST_PP_ADD(BOOST_PP_SUB(NB_BRANCH, i),   \
                                                 BOOST_PP_SUB(NB_BRANCH, 1)))  \
        BOOST_PP_REPEAT(BOOST_PP_ADD(BOOST_PP_SUB(NB_BRANCH, i), 1),           \
                        FIELD_ADD_ACC, a)                                      \
        BOOST_PP_REPEAT(BOOST_PP_SUB(NB_BRANCH, i), FIELD_MUL_ACC_VEC_REV,     \
                        k + BOOST_PP_SUB(BOOST_PP_SUB(NB_BRANCH, 1), i))       \
        BOOST_PP_REPEAT_FROM_TO(1, NB_BRANCH, REDUCE_ACCUMULATORS, acc)        \
    } else
            // end IF_MORE_THAN_N_REMAINING_BLOCKS
            BOOST_PP_REPEAT_FROM_TO(1, BOOST_PP_ADD(NB_BRANCH, 1),
                                    IF_MORE_THAN_N_REMAINING_BLOCKS, 0) {}
            pack_field_elem((baseint_t *)tag_packed, acc);
        }
    }
    transform_field_elem(out, OUTPUTSIZE, tag_packed, BUFFSIZE);
}
