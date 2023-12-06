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

typedef struct EL_int1163_single {
    uint64_t val[3];
} EL_field_elem_t;
typedef struct EL_int1163_single_p {
    uint64_t val[2][2];
} EL_field_elem_precomputed_t;

static inline __attribute__((always_inline)) int
EL_precompute_factor(EL_field_elem_precomputed_t *res, field_elem_t *b) {
    res->val[0][0] = b->val[0];
    res->val[0][1] = b->val[1];
    res->val[1][0] = ((uint64_t)b->val[0] * (3));
    res->val[1][1] = ((uint64_t)b->val[1] * (3));
    return 0;
}

static inline __attribute__((always_inline)) int
EL_field_mul_precomputed_no_carry(dfield_elem_t *res, EL_field_elem_t *a,
                                  EL_field_elem_precomputed_t *b) {
    res->val[0] = ((uint128_t)a->val[0] * b->val[0][0]);
    res->val[0] += ((uint128_t)a->val[1] * b->val[1][1]);
    res->val[0] += ((uint128_t)a->val[2] * b->val[1][0]);

    res->val[1] = ((uint128_t)a->val[0] * b->val[0][1]);
    res->val[1] += ((uint128_t)a->val[1] * b->val[0][0]);
    res->val[1] += ((uint128_t)a->val[2] * b->val[1][1]);
    return 0;
}

static inline __attribute__((always_inline)) int
EL_carry_round(EL_field_elem_t *res, dfield_elem_t *a) {
    res->val[0] = ((uint64_t)(a->val[0])) & (((((uint64_t)1) << 58) - 1));
    a->val[1] += (uint64_t)((a->val[0]) >> 58);

    res->val[2] = (uint64_t)((a->val[1]) >> 58);
    res->val[1] = ((uint64_t)(a->val[1])) & (((((uint64_t)1) << 58) - 1));
    return 0;
}

// classical horner with modified arithmetic with one extra limb to make carry
// faster (but multiplication slower)
void classical_Horner_PK_1B_Delay_a_3EL(unsigned char *out,
                                        const unsigned char *in,
                                        unsigned long long inlen,
                                        const unsigned char *key) {
    if (inlen == 0) {
        memset(out, 0, OUTPUTSIZE);
        return;
    }
    EL_field_elem_t acc_el = {0}; // accumulator for horner algorithm
    field_elem_t acc = {0};
    dfield_elem_t acc_d = {0}; // accumulator for horner algorithm
    field_elem_t
        a; // temporary field element representing the block being processed
    field_elem_t k; // univariate key
    EL_field_elem_precomputed_t k_p;
    unsigned char transkey[BUFFSIZE] = {0};
    unsigned char tag_packed[BUFFSIZE] = {0};

    // Transform key from a byte array to one field elements
    transform_key(transkey, BUFFSIZE, key, KEYSIZE);
    unpack_field_elem(&k, (baseint_t *)transkey);
    EL_precompute_factor(&k_p, &k);

    // processing all blocks except the last one (possibly smaller)
    while (inlen > BLOCKSIZE) {
        unpack_and_encode_field_elem(&a, (baseint_t *)in);
        field_add_mix(&acc_d, &acc_d, &a);
        EL_carry_round(&acc_el, &acc_d);
        EL_field_mul_precomputed_no_carry(&acc_d, &acc_el, &k_p);
        inlen -= BLOCKSIZE;
        in += BLOCKSIZE;
    }
    // processing last block
    if (inlen) {
        unpack_and_encode_last_field_elem(&a, (baseint_t *)in, inlen);
        field_add_mix(&acc_d, &acc_d, &a);
        carry_round(&acc, &acc_d);
        reduce(&acc, &acc);
    }
    pack_field_elem((baseint_t *)tag_packed,
                    &acc); // transform limb representation of field element
                           // (unpacked) into packed field element
    transform_field_elem(
        out, OUTPUTSIZE, tag_packed,
        BUFFSIZE); // transform from field element to byte missing?
}
