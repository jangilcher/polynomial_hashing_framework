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

typedef struct int1305_quad {
    uint128_t val[6];
} tradix_field_elem_t;

inline __attribute__((always_inline)) void
unpack_field_elem_bis(field_elem_t *res, uint64_t *a) {
    res->val[0] = a[0];
    res->val[1] = a[1];
}

inline __attribute__((always_inline)) void
carry_round_Tradix(field_elem_t *res, tradix_field_elem_t *a) {
    // uint128_t c;
    uint128_t acc;
    acc = a->val[3];
    a->val[1] += (((uint64_t)acc) & (((((uint64_t)1) << 24) - 1))) << 20;
    a->val[2] += (uint128_t)(acc >> 24);

    acc = a->val[4];
    a->val[2] += (((uint64_t)acc) & (((((uint64_t)1) << 22) - 1))) << 20;
    a->val[0] += ((uint128_t)(acc >> 22) * 5);

    acc = a->val[5];
    a->val[0] += (((uint64_t)acc) & (((((uint64_t)1) << 22) - 1))) << 22;
    a->val[1] += (uint128_t)(acc >> 22);

    acc = (uint128_t)(a->val[0] >> (44));
    res->val[0] = ((uint64_t)(a->val[0])) & (((((uint64_t)1) << 44) - 1));
    a->val[1] += acc;
    acc = (uint128_t)(a->val[1] >> (44));
    res->val[1] = ((uint64_t)(a->val[1])) & (((((uint64_t)1) << 44) - 1));
    a->val[2] += acc;
    acc = (uint128_t)(a->val[2] >> (42));
    res->val[2] = ((uint64_t)(a->val[2])) & (((((uint64_t)1) << 42) - 1));
    // res->val[0] += c * 5;
    acc = (uint128_t)(res->val[0] + acc * 5);
    res->val[0] = ((uint64_t)(acc)) & (((((uint64_t)1) << 44) - 1));
    // c = (uint128_t) (res->val[0] >> (44));
    res->val[1] += (uint64_t)(acc >> (44));
}

inline __attribute__((always_inline)) void
field_mul_no_carry_Tradix(tradix_field_elem_t *res, field_elem_t *a,
                          field_elem_t *b) {
    res->val[0] = ((uint128_t)a->val[0] * b->val[0]);
    res->val[1] = ((uint128_t)a->val[1] * b->val[0]);
    res->val[2] = ((uint128_t)a->val[2] * b->val[0]);
    res->val[3] = ((uint128_t)a->val[0] * b->val[1]);
    res->val[4] = ((uint128_t)a->val[1] * b->val[1]);
    res->val[5] = ((uint128_t)a->val[2] * b->val[1] * 5);
}

inline __attribute__((always_inline)) void
field_mul_precomputed_no_carry_Tradix(tradix_field_elem_t *res, field_elem_t *a,
                                      field_elem_precomputed_t *b) {
    res->val[0] = ((uint128_t)b->val[0][0][0] * a->val[0]);
    res->val[1] = ((uint128_t)b->val[0][1][1] * a->val[0]);
    res->val[2] = ((uint128_t)b->val[0][2][2] * a->val[0]);
    res->val[3] = ((uint128_t)b->val[0][0][0] * a->val[1]);
    res->val[4] = ((uint128_t)b->val[0][1][1] * a->val[1]);
    res->val[5] = ((uint128_t)(b->val[2][2][1] >> 2) *
                   a->val[1]); // this can be improved in precomputed values to
                               // remove the shift
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

// reduction after last multiplication
void classical_Naive_PK_Delay_b_2radix(unsigned char *out,
                                       const unsigned char *in,
                                       unsigned long long inlen,
                                       const unsigned char *key) {
    if (inlen == 0) {
        memset(out, 0, OUTPUTSIZE);
        return;
    }
    unsigned long long noOfBlocks =
        inlen / BLOCKSIZE + (inlen % BLOCKSIZE != 0);
    const unsigned char *in_temp;
    field_elem_t acc = {0}; // accumulator for horner algorithm
    tradix_field_elem_t acc_d, a_d;
    field_elem_t
        a; // temporary field element representing the block being processed
    field_elem_t k[2]; // univariate key
    field_elem_precomputed_t k_p;
    unsigned char buff[BUFFSIZE] = {0};
    unsigned char transkey[BUFFSIZE] = {0};
    unsigned char tag_packed[BUFFSIZE] = {0}; // what is this size?

    // Transform key from a byte array to one field elements
    transform_key(transkey, BUFFSIZE, key,
                  KEYSIZE); // transform key from bytes to a packed field
                            // elements of BLOCKSIZE bytes
    unpack_field_elem(k,
                      (baseint_t *)transkey); // transform field element(packed)
                                              // into limb representation
    precompute_factor(&k_p, k);

    in_temp = in + (noOfBlocks - 1) * BLOCKSIZE;

    // process second last block
    if (noOfBlocks > 1) {
        in_temp -= BLOCKSIZE;
        transform_msg(buff, BUFFSIZE, in_temp, BLOCKSIZE);
        unpack_field_elem_bis(&a, (baseint_t *)buff);
        field_mul_precomputed_no_carry_Tradix(&acc_d, &a, &k_p);
        // field_add_mix(&acc_d, &a_d, &acc);

        // process the rest of the msg
        if (noOfBlocks > 2) {
            in_temp -= BLOCKSIZE;
            transform_msg(buff, BUFFSIZE, in_temp,
                          BLOCKSIZE); // transform msg block from bytes to field
                                      // elements (packed)
            unpack_field_elem_bis(
                &a, (baseint_t *)buff); // transform field element(packed) into
                                        // limb representation
            field_mul(k + 1, k, k);     // <<< could use precomputation
            field_mul_no_carry_Tradix(&a_d, k + 1, &a);
            field_add_dbl_Tradix(&acc_d, &acc_d, &a_d);
            while (in_temp != in) {
                in_temp -= BLOCKSIZE;
                transform_msg(buff, BUFFSIZE, in_temp,
                              BLOCKSIZE); // transform msg block from bytes to
                                          // field elements (packed)
                unpack_field_elem_bis(
                    &a, (baseint_t *)buff); // transform field element(packed)
                                            // into limb representation
                field_mul_precomputed(k + 1, k + 1, &k_p);
                field_mul_no_carry_Tradix(&a_d, k + 1, &a);
                field_add_dbl_Tradix(&acc_d, &acc_d, &a_d);
            }
        }
        carry_round_Tradix(&acc, &acc_d);
    }
    // process last block
    transform_msg(buff, BUFFSIZE, in + (noOfBlocks - 1) * BLOCKSIZE,
                  inlen - (noOfBlocks - 1) *
                              BLOCKSIZE); // transform msg block from bytes to
                                          // field elements (packed)
    unpack_field_elem(
        &a,
        (baseint_t *)
            buff); // transform field element(packed) into limb representation

    field_add(&acc, &acc, &a);
    _carry_round(&acc, &acc);
    reduce(&acc, &acc);
    pack_field_elem((baseint_t *)tag_packed,
                    &acc); // transform limb representation of field element
                           // (unpacked) into packed field element
    transform_field_elem(
        out, OUTPUTSIZE, tag_packed,
        BUFFSIZE); // transform from field element to byte missing?
}
