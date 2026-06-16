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
#include "boost/preprocessor/arithmetic/mul.hpp"
#include "boost/preprocessor/arithmetic/sub.hpp"
#include "boost/preprocessor/repetition/repeat.hpp"
#include <stddef.h>
#include <string.h>

#if defined(INNERPOLY_H) && defined(INNERPOLY)
#include INNERPOLY_H
#endif

// Number of blocks at the lower level
#ifdef OUTER_PARAM0
#define NB_BLOCK_FLVL OUTER_PARAM0
#else
#define NB_BLOCK_FLVL 1
#endif

// Indicate if reduction after each multiplication (when=1) or at the end of
// lower level last multiplication
#ifdef OUTER_PARAM1
#define REDUCTION OUTER_PARAM1
#else
#define REDUCTION 0
#endif

// PARAM2 indicates the usage of the same accumulator inside the inner loop
// (PARAM2=1) or the use of multiple parallel ones (PARAM2=0)
#ifdef OUTER_PARAM2
#define USE_SAME_ACC OUTER_PARAM3
#else
#define USE_SAME_ACC 0
#endif

// Indicate if reduction after each multiplication (when=1) or at the end of
// lower level last multiplication
#if REDUCTION
#define DEFINE_D_ACC field_elem_t a_d[nbBlockFlvl];
#define CARRY_D_ACC
#if USE_SAME_ACC
#define MULT_AND_ACC                                                           \
    field_mul(a_d, a, a + 1);                                                  \
    field_add(&acc, &acc, a_d);
#else
#define MULT_AND_ACC                                                           \
    field_mul(a_d + j, a + j, a + j + 1);                                      \
    field_add(&acc, &acc, a_d + j);
#endif
#define ADD_MULT_AND_ACC_BOOST(z, j, a)                                        \
    field_add(a + BOOST_PP_MUL(j, 2), a + BOOST_PP_MUL(j, 2),                  \
              k + BOOST_PP_MUL(j, 2));                                         \
    field_add(a + BOOST_PP_ADD(BOOST_PP_MUL(j, 2), 1),                         \
              a + BOOST_PP_ADD(BOOST_PP_MUL(j, 2), 1),                         \
              k + BOOST_PP_ADD(BOOST_PP_MUL(j, 2), 1));                        \
    field_mul(a_d + BOOST_PP_MUL(j, 2), a + BOOST_PP_MUL(j, 2),                \
              a + BOOST_PP_ADD(BOOST_PP_MUL(j, 2), 1));                        \
    field_add(&acc, &acc, a_d + BOOST_PP_MUL(j, 2));
#define MULT_NEXT field_mul(&acc, &acc, k + nbBlockFlvl - 1);
#else
#define DEFINE_D_ACC                                                           \
    dfield_elem_t acc_d = {0};                                                 \
    dfield_elem_t a_d[nbBlockFlvl];
#define CARRY_D_ACC carry_round(&acc, &acc_d);
#if USE_SAME_ACC
#define MULT_AND_ACC                                                           \
    field_mul_no_carry(a_d, a, a + 1);                                         \
    field_add_dbl(&acc_d, &acc_d, a_d);
#else
#define MULT_AND_ACC                                                           \
    field_mul_no_carry(a_d + j, a + j, a + j + 1);                             \
    field_add_dbl(&acc_d, &acc_d, a_d + j);
#endif
#define ADD_MULT_AND_ACC_BOOST(z, j, a)                                        \
    field_add(a + BOOST_PP_MUL(j, 2), a + BOOST_PP_MUL(j, 2),                  \
              k + BOOST_PP_MUL(j, 2));                                         \
    field_add(a + BOOST_PP_ADD(BOOST_PP_MUL(j, 2), 1),                         \
              a + BOOST_PP_ADD(BOOST_PP_MUL(j, 2), 1),                         \
              k + BOOST_PP_ADD(BOOST_PP_MUL(j, 2), 1));                        \
    field_mul_no_carry(a_d + BOOST_PP_MUL(j, 2), a + BOOST_PP_MUL(j, 2),       \
                       a + BOOST_PP_ADD(BOOST_PP_MUL(j, 2), 1));               \
    field_add_dbl(&acc_d, &acc_d, a_d + BOOST_PP_MUL(j, 2));
#define MULT_NEXT field_mul_no_carry(&acc_d, &acc, k + nbBlockFlvl - 1);
#endif

#define IS_EVEN(x) ((x) % 2 == 0)
#if IS_EVEN(NB_BLOCK_FLVL)
#define ADD_ODD_BLOCK
#else
#define ADD_ODD_BLOCK field_add(&acc, &acc, a + nbBlockFlvl - 1);
#endif

#define UNPACK_AND_ENCODE(z, j, a)                                             \
    unpack_and_encode_field_elem(a + j, (baseint_t *)in);                      \
    in += BLOCKSIZE;

// reduction after last multiplication
void v1NMH_Horner_2level_NB_Delay_b_test(unsigned char *out,
                                         const unsigned char *in,
                                         unsigned long long inlen,
                                         const unsigned char *key,
                                         unsigned long long keylen) {
    if (inlen == 0) {
        memset(out, 0, OUTPUTSIZE);
        return;
    }
    unsigned int nbBlockFlvl = NB_BLOCK_FLVL;
    unsigned int i = 0, j = 0;
    field_elem_t acc = {0};
    DEFINE_D_ACC;
    field_elem_t a[nbBlockFlvl]; // temporary field element representing the
                                 // block being processed
    field_elem_t k[nbBlockFlvl];
    unsigned char tag_packed[BUFFSIZE] = {0};
    unsigned long long nbBlockFlvl_blocksize = nbBlockFlvl * BLOCKSIZE;

    // Transform key from a byte array to one field elements
    unpack_and_encode_key(k, (baseint_t *)key);

    // process msg of only 1 block
    if (inlen <= BLOCKSIZE) {
        // transform_msg(tag_packed, BUFFSIZE, in, inlen); // transform msg
        // block from bytes to field elements (packed)
        unpack_and_encode_last_field_elem(&acc, (baseint_t *)in, inlen);
        pack_field_elem((baseint_t *)tag_packed, &acc);
    } else if (inlen <= nbBlockFlvl_blocksize) { // process msg of nbBlockFlvl
                                                 // Blocks or less
        for (i = 2; i < nbBlockFlvl + 1; ++i) {
            if (inlen <= i * BLOCKSIZE) {
                // compute key powers
                for (j = 0; j < i - 1; ++j) {
                    field_mul(k + j + 1, k + j, k);
                }
                for (j = 0; j < i - 1; ++j) {
                    unpack_and_encode_field_elem(a + j, (baseint_t *)in);
                    in += BLOCKSIZE;
                    inlen -= BLOCKSIZE;
                }
                unpack_and_encode_last_field_elem(a + i - 1, (baseint_t *)in,
                                                  inlen);

                for (j = 0; j < i - 1; j += 2) {
                    field_add(a + j, a + j, k + j);
                    field_add(a + j + 1, a + j + 1, k + j + 1);
                    MULT_AND_ACC;
                }
                CARRY_D_ACC;
                if (j == i - 1) {
                    field_add(&acc, &acc, a + j);
                    _carry_round(&acc, &acc);
                }
                reduce(&acc, &acc);
                pack_field_elem((baseint_t *)tag_packed, &acc);
                break;
            }
        }
    } else { // process msg of more than nbBlockFlvl Blocks
        // if (inlen > nbBlockFlvl_blocksize) {
        // compute key powers for parallel horner
        for (j = 0; j < nbBlockFlvl - 1; ++j) {
            // field_mul(k+i+1, k+i ,k);
            field_mul(k + j + 1, k + j, k);
        }
        // initialize accumulator
#if USE_SAME_ACC
        for (j = 0; j < nbBlockFlvl - 1; j += 2) {
            unpack_and_encode_field_elem(a, (baseint_t *)in);
            in += BLOCKSIZE;
            unpack_and_encode_field_elem(a + 1, (baseint_t *)in);
            in += BLOCKSIZE;
            field_add(a, a, k + j);
            field_add(a + 1, a + 1, k + j + 1);
            MULT_AND_ACC;
        }
        inlen -= nbBlockFlvl_blocksize;
#else
        //        for (j = 0; j < nbBlockFlvl; ++j) {
        //            unpack_and_encode_field_elem(a + j, (baseint_t *)in);
        //            in += BLOCKSIZE;
        //        }
        BOOST_PP_REPEAT(NB_BLOCK_FLVL, UNPACK_AND_ENCODE, a);
        inlen -= nbBlockFlvl_blocksize;
        //        for (j = 0; j < nbBlockFlvl - 1; j += 2) {
        //            field_add(a + j, a + j, k + j);
        //            field_add(a + j + 1, a + j + 1, k + j + 1);
        //            MULT_AND_ACC;
        //        }
        BOOST_PP_REPEAT(BOOST_PP_DIV(NB_BLOCK_FLVL, 2), ADD_MULT_AND_ACC_BOOST,
                        a);
#endif
        CARRY_D_ACC;
        ADD_ODD_BLOCK;

        while (inlen > nbBlockFlvl_blocksize) {
#if USE_SAME_ACC
            MULT_NEXT;
            for (j = 0; j < nbBlockFlvl - 1; j += 2) {
                unpack_and_encode_field_elem(a, (baseint_t *)in);
                in += BLOCKSIZE;
                unpack_and_encode_field_elem(a + 1, (baseint_t *)in);
                in += BLOCKSIZE;
                field_add(a, a, k + j);
                field_add(a + 1, a + 1, k + j + 1);
                MULT_AND_ACC;
            }
            inlen -= nbBlockFlvl_blocksize;
#else
            //            for (j = 0; j < nbBlockFlvl; ++j) {
            //                unpack_and_encode_field_elem(a + j, (baseint_t
            //                *)in); in += BLOCKSIZE;
            //            }
            BOOST_PP_REPEAT(NB_BLOCK_FLVL, UNPACK_AND_ENCODE, a);
            inlen -= nbBlockFlvl_blocksize;

            MULT_NEXT;
            //            for (j = 0; j < nbBlockFlvl - 1; j += 2) {
            //                field_add(a + j, a + j, k + j);
            //                field_add(a + j + 1, a + j + 1, k + j + 1);
            //                MULT_AND_ACC;
            //            }
            BOOST_PP_REPEAT(BOOST_PP_DIV(NB_BLOCK_FLVL, 2),
                            ADD_MULT_AND_ACC_BOOST, a);
#endif
            CARRY_D_ACC;
            ADD_ODD_BLOCK;
        }
        // process remaining msg of length less or equal to nbBlockFlvl Blocks
        for (i = 0; i < nbBlockFlvl; ++i) {
            if (inlen > (nbBlockFlvl - i - 1) * BLOCKSIZE) {
                for (j = 0; j < nbBlockFlvl - i - 1; ++j) {
                    unpack_and_encode_field_elem(a + j, (baseint_t *)in);
                    in += BLOCKSIZE;
                    inlen -= BLOCKSIZE;
                }
                unpack_and_encode_last_field_elem(a + nbBlockFlvl - i - 1,
                                                  (baseint_t *)in, inlen);

                MULT_NEXT;
                for (j = 0; j < nbBlockFlvl - i - 1; j += 2) {
                    field_add(a + j, a + j, k + j);
                    field_add(a + j + 1, a + j + 1, k + j + 1);
                    MULT_AND_ACC;
                }
                CARRY_D_ACC;
                if (j == nbBlockFlvl - i - 1) {
                    field_add(&acc, &acc, a + j);
                    _carry_round(&acc, &acc);
                }
                break;
            }
        }

        reduce(&acc, &acc);
        pack_field_elem((baseint_t *)tag_packed,
                        &acc); // transform limb representation of field element
                               // (unpacked) into packed field element
    }
    transform_field_elem(
        out, OUTPUTSIZE, tag_packed,
        BUFFSIZE); // transform from field element to byte missing?
}
