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
#include <stddef.h>
#include <string.h>

#if defined(INNERPOLY_H) && defined(INNERPOLY)
#include INNERPOLY_H
#endif

#define NB_BLOCK_FLVL 3

typedef struct int1305_quad {
    uint128_t val[6];
} tradix_field_elem_t;

inline __attribute__((always_inline)) void
unpack_field_elem_bis(field_elem_t *res, uint64_t *a) {
    res->val[0] = a[0];
    res->val[1] = a[1];
    res->val[2] = a[2] & ((((uint64_t)1) << 40) - 1);
}

inline __attribute__((always_inline)) void
carry_round_Tradix(field_elem_t *res, tradix_field_elem_t *a) {
    // uint128_t c;
    uint128_t acc;
    acc = a->val[3];
    a->val[1] += (((uint64_t)acc) & (((((uint64_t)1) << 52) - 1))) << 6;
    a->val[2] += (uint128_t)(acc >> 52);

    acc = a->val[4];
    a->val[2] += (((uint64_t)acc) & (((((uint64_t)1) << 52) - 1))) << 6;
    a->val[0] += ((uint128_t)(acc >> 52) * 3);

    acc = a->val[5];
    a->val[0] += (((uint64_t)acc) & (((((uint64_t)1) << 52) - 1))) << 6;
    a->val[1] += (uint128_t)(acc >> 52);

    acc = (uint128_t)(a->val[0] >> (58));
    res->val[0] = ((uint64_t)(a->val[0])) & (((((uint64_t)1) << 58) - 1));
    a->val[1] += ((uint128_t)acc);
    acc = (uint128_t)(a->val[1] >> (58));
    res->val[1] = ((uint64_t)(a->val[1])) & (((((uint64_t)1) << 58) - 1));
    a->val[2] += ((uint128_t)acc);
    acc = (uint128_t)(a->val[2] >> (58));
    res->val[2] = ((uint64_t)(a->val[2])) & (((((uint64_t)1) << 58) - 1));
    // res->val[0] += c * 5;
    acc = (uint128_t)(res->val[0] + acc * 3);
    res->val[0] = ((uint64_t)(acc)) & (((((uint64_t)1) << 58) - 1));
    // c = (uint128_t) (res->val[0] >> (44));
    res->val[1] += (uint64_t)(acc >> (58));
}

inline __attribute__((always_inline)) void
field_mul_precomputed_no_carry_Tradix(tradix_field_elem_t *res, field_elem_t *a,
                                      field_elem_precomputed_t *b) {
    res->val[0] = ((uint128_t)b->val[0][0][0] * a->val[0]);
    res->val[1] = ((uint128_t)b->val[0][1][1] * a->val[0]);
    res->val[2] = ((uint128_t)b->val[0][2][2] * a->val[0]);
    res->val[3] = ((uint128_t)b->val[0][0][0] * a->val[1]);
    res->val[4] = ((uint128_t)b->val[0][1][1] * a->val[1]);
    res->val[5] = ((uint128_t)b->val[2][2][1] *
                   a->val[1]); // this can be improved in precomputed values to
                               // remove the shift
}

inline __attribute__((always_inline)) void
field_mul_precomputed_no_carry_bis(tradix_field_elem_t *res, field_elem_t *a,
                                   field_elem_precomputed_t *b) {
    uint128_t acc;
    res->val[0] = 0;
    acc = ((uint128_t)a->val[0] * b->val[0][0][0]);
    res->val[0] += acc;
    acc = ((uint128_t)a->val[1] * b->val[1][2][0]);
    res->val[0] += acc;
    acc = ((uint128_t)a->val[2] * b->val[2][1][0]);
    res->val[0] += acc;

    res->val[1] = 0;
    acc = ((uint128_t)a->val[0] * b->val[0][1][1]);
    res->val[1] += acc;
    acc = ((uint128_t)a->val[1] * b->val[1][0][1]);
    res->val[1] += acc;
    acc = ((uint128_t)a->val[2] * b->val[2][2][1]);
    res->val[1] += acc;

    res->val[2] = 0;
    acc = ((uint128_t)a->val[0] * b->val[0][2][2]);
    res->val[2] += acc;
    acc = ((uint128_t)a->val[1] * b->val[1][1][2]);
    res->val[2] += acc;
    acc = ((uint128_t)a->val[2] * b->val[2][0][2]);
    res->val[2] += acc;

    res->val[3] = 0;
    res->val[4] = 0;
    res->val[5] = 0;
}

inline __attribute__((always_inline)) void
field_add_dbl_Tradix(tradix_field_elem_t *res, tradix_field_elem_t *a,
                     tradix_field_elem_t *b) {
    res->val[0] = a->val[0] + b->val[0];
    res->val[1] = a->val[1] + b->val[1];
    res->val[2] = a->val[2] + b->val[2];
    res->val[3] = a->val[3] + b->val[3];
    res->val[4] = a->val[4] + b->val[4];
    res->val[5] = a->val[5] + b->val[5];
}

// Classical 2 level with reduction after last multiplication
void classical_2level_3B_PK_Delay_b_2radix_1743(unsigned char *out,
                                                const unsigned char *in,
                                                unsigned long long inlen,
                                                const unsigned char *key) {
    if (inlen == 0) {
        memset(out, 0, OUTPUTSIZE);
        return;
    }
    unsigned int i, j;
    field_elem_t acc = {0};
    tradix_field_elem_t acc_d[NB_BLOCK_FLVL];
    field_elem_t a[NB_BLOCK_FLVL]; // temporary field element representing the
                                   // block being processed
    field_elem_t k[NB_BLOCK_FLVL];
    field_elem_precomputed_t k_p[NB_BLOCK_FLVL];
    // unsigned char buff[NB_BLOCK_FLVL][BUFFSIZE];
    unsigned char transkey[BUFFSIZE] = {0};
    unsigned char tag_packed[BUFFSIZE] = {0};
    unsigned long long nbBlockFlvl_blocksize = NB_BLOCK_FLVL * BLOCKSIZE;

    // Transform key from a byte array to one field elements
    transform_key(transkey, BUFFSIZE, key,
                  KEYSIZE); // transform key from bytes to a packed field
                            // elements of BLOCKSIZE bytes
    unpack_field_elem(k,
                      (baseint_t *)transkey); // transform field element(packed)
                                              // into limb representation
    precompute_factor(k_p, k);

    // process msg of only 1 block
    if (inlen <= BLOCKSIZE) {
        // transform_msg(tag_packed, BUFFSIZE, in, inlen); // transform msg
        // block from bytes to field elements (packed)
        unpack_and_encode_last_field_elem(&acc, (baseint_t *)in, inlen);
        pack_field_elem((baseint_t *)tag_packed, &acc);
    } else if (inlen <= nbBlockFlvl_blocksize) { // process msg of NB_BLOCK_FLVL
                                                 // Blocks or less
        for (i = 2; i < NB_BLOCK_FLVL + 1; ++i) {
            if (inlen <= i * BLOCKSIZE) {
                // compute key powers
                for (j = 0; j < i - 2; ++j) {
                    field_mul_precomputed(k + j + 1, k + j, k_p);
                    precompute_factor(k_p + j + 1, k + j + 1);
                }
                for (j = 0; j < i - 1; ++j) {
                    unpack_field_elem_bis(
                        a + j,
                        (baseint_t *)in); // transform field element(packed)
                                          // into limb representation
                    in += BLOCKSIZE;
                    inlen -= BLOCKSIZE;
                }
                unpack_and_encode_last_field_elem(a + i - 1, (baseint_t *)in,
                                                  inlen);

                field_mul_precomputed_no_carry_Tradix(acc_d, a, k_p + i - 2);
                for (j = 1; j < i - 1; ++j) {
                    field_mul_precomputed_no_carry_Tradix(acc_d + j, a + j,
                                                          k_p + i - 2 - j);
                    field_add_dbl_Tradix(acc_d, acc_d, acc_d + j);
                }
                carry_round_Tradix(&acc, acc_d);
                field_add(&acc, &acc, a + i - 1);
                _carry_round(&acc, &acc);
                reduce(&acc, &acc);
                pack_field_elem((baseint_t *)tag_packed, &acc);
                break;
            }
        }
    } else { // process msg of more than NB_BLOCK_FLVL Blocks
        // if (inlen > NB_BLOCK_FLVL_blocksize) {
        // compute key powers for parallel horner
        for (i = 0; i < NB_BLOCK_FLVL - 1; ++i) {
            field_mul_precomputed(k + i + 1, k + i, k_p);
            // carry_round(k+i+1, acc_d);
            precompute_factor(k_p + i + 1, k + i + 1);
        }
        // initialize accumulator
        for (i = 0; i < NB_BLOCK_FLVL - 1; ++i) {
            unpack_field_elem_bis(a + i, (baseint_t *)in);
            in += BLOCKSIZE;
        }
        unpack_and_encode_field_elem(a + NB_BLOCK_FLVL - 1, (baseint_t *)in);
        in += BLOCKSIZE;

        inlen -= nbBlockFlvl_blocksize;
        field_mul_precomputed_no_carry_Tradix(acc_d, a,
                                              k_p + NB_BLOCK_FLVL - 2);
        for (i = 1; i < NB_BLOCK_FLVL - 1; ++i) {
            field_mul_precomputed_no_carry_Tradix(acc_d + i, a + i,
                                                  k_p + NB_BLOCK_FLVL - 2 - i);
            field_add_dbl_Tradix(acc_d, acc_d, acc_d + i);
        }
        carry_round_Tradix(&acc, acc_d);
        field_add(&acc, &acc, a + NB_BLOCK_FLVL - 1);

        while (inlen > nbBlockFlvl_blocksize) {
            for (i = 0; i < NB_BLOCK_FLVL - 1; ++i) {
                unpack_field_elem_bis(a + i, (baseint_t *)in);
                in += BLOCKSIZE;
            }
            unpack_and_encode_field_elem(a + NB_BLOCK_FLVL - 1,
                                         (baseint_t *)in);
            in += BLOCKSIZE;
            inlen -= nbBlockFlvl_blocksize;

            field_mul_precomputed_no_carry_bis(acc_d, &acc,
                                               k_p + NB_BLOCK_FLVL - 1);
            for (i = 0; i < NB_BLOCK_FLVL - 1; ++i) {
                field_mul_precomputed_no_carry_Tradix(
                    acc_d + i + 1, a + i, k_p + NB_BLOCK_FLVL - 2 - i);
                field_add_dbl_Tradix(acc_d, acc_d, acc_d + i + 1);
            }
            carry_round_Tradix(&acc, acc_d);
            field_add(&acc, &acc, a + NB_BLOCK_FLVL - 1);
        }
        // process remaining msg of length less or equal to NB_BLOCK_FLVL Blocks
        for (i = 0; i < NB_BLOCK_FLVL; ++i) {
            if (inlen > (NB_BLOCK_FLVL - i - 1) * BLOCKSIZE) {
                for (j = 0; j < NB_BLOCK_FLVL - i - 1; ++j) {
                    unpack_field_elem_bis(
                        a + j,
                        (baseint_t *)in); // transform field element(packed)
                                          // into limb representation
                    in += BLOCKSIZE;
                    inlen -= BLOCKSIZE;
                }
                unpack_and_encode_last_field_elem(a + NB_BLOCK_FLVL - i - 1,
                                                  (baseint_t *)in, inlen);

                field_mul_precomputed_no_carry_bis(acc_d, &acc,
                                                   k_p + NB_BLOCK_FLVL - i - 1);
                for (j = 0; j < NB_BLOCK_FLVL - i - 1; ++j) {
                    field_mul_precomputed_no_carry_Tradix(
                        acc_d + j + 1, a + j, k_p + NB_BLOCK_FLVL - i - 2 - j);
                    field_add_dbl_Tradix(acc_d, acc_d, acc_d + j + 1);
                }
                carry_round_Tradix(&acc, acc_d);
                field_add(&acc, &acc, a + NB_BLOCK_FLVL - i - 1);
                _carry_round(&acc, &acc);
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
