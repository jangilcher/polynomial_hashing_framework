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

inline __attribute__((always_inline)) void
unpack_field_elem_KC(field_elem_t *res, uint64_t *a) {
    res->val[0] = a[0];
    res->val[1] = a[1];
    res->val[2] = a[2];
}

inline __attribute__((always_inline)) void pack_field_elem_KC(uint64_t *res,
                                                              field_elem_t *a) {
    res[0] = (a->val[0]);
    res[1] = (a->val[1]);
    res[2] = (a->val[2]);
}

inline __attribute__((always_inline)) void
field_mul_no_carry_KC(dfield_elem_t *res, field_elem_t *a, field_elem_t *b) {
    uint128_t acc;

    acc = ((uint128_t)a->val[0] * b->val[0]);
    res->val[0] = acc;
    acc = ((uint128_t)((a->val[2] * b->val[0] +
                        (uint128_t)a->val[1] * b->val[1]) >>
                       2) *
           5);
    res->val[0] += acc;

    acc = ((uint128_t)a->val[1] * b->val[0]);
    res->val[1] = acc;
    acc = ((uint128_t)a->val[0] * b->val[1]);
    res->val[1] += acc;
    acc = ((a->val[2] * b->val[1]) >> 2) * 5;
    res->val[1] += acc;

    res->val[2] = 0;
}

inline __attribute__((always_inline)) void carry_round_KC(field_elem_t *res,
                                                          dfield_elem_t *a) {
    uint64_t c;

    // need to check how many rounds are necessary
    c = (uint64_t)(a->val[0] >> (64));
    a->val[1] += c;
    a->val[0] = (uint64_t)a->val[0];

    c = (uint64_t)(a->val[1] >> (64));
    res->val[1] = ((uint64_t)(a->val[1]));
    a->val[2] += c;
    c = (uint64_t)(a->val[2] >> (2));
    res->val[2] = ((uint64_t)(a->val[2])) & (((((uint64_t)1) << 2) - 1));

    a->val[0] += c * 5;
    res->val[0] = ((uint64_t)(a->val[0]));
    c = (a->val[0] >> 64);
    res->val[1] += c;
}

// classical horner with reduction after addition only
void classical_Horner_1B_Delay_a_KC(unsigned char *out, const unsigned char *in,
                                    unsigned long long inlen,
                                    const unsigned char *key) {
    if (inlen == 0) {
        memset(out, 0, OUTPUTSIZE);
        return;
    }
    field_elem_t acc = {0};    // accumulator for horner algorithm
    dfield_elem_t acc_d = {0}; // accumulator for horner algorithm
    field_elem_t
        a; // temporary field element representing the block being processed
    field_elem_t k; // univariate key
    unsigned char buff[BUFFSIZE] = {0};
    unsigned char transkey[BUFFSIZE] = {0};
    unsigned char tag_packed[BUFFSIZE] = {0}; // what is this size?

    // Transform key from a byte array to one field elements
    transform_key(transkey, BUFFSIZE, key,
                  KEYSIZE); // transform key from bytes to a packed field
                            // elements of BLOCKSIZE bytes
    unpack_field_elem_KC(
        &k, (baseint_t *)transkey); // transform field element(packed) into limb
                                    // representation

    // processing all blocks except the last one (possibly smaller)
    while (inlen > BLOCKSIZE) {
        transform_msg(buff, BUFFSIZE, in,
                      BLOCKSIZE); // transform msg block from bytes to field
                                  // elements (packed)
        unpack_field_elem_KC(
            &a, (baseint_t *)buff); // transform field element(packed) into limb
                                    // representation
        field_add_mix(&acc_d, &acc_d, &a);
        carry_round_KC(&acc, &acc_d);
        field_mul_no_carry_KC(&acc_d, &acc, &k);
        inlen -= BLOCKSIZE;
        in += BLOCKSIZE;
    }
    // processing last block
    if (inlen) {
        // memset(buff, 0, BUFFSIZE);
        transform_msg(
            buff, BUFFSIZE, in,
            inlen); // transform msg block from bytes to field elements (packed)
        unpack_field_elem_KC(
            &a, (baseint_t *)buff); // transform field element(packed) into limb
                                    // representation
        field_add_mix(&acc_d, &acc_d, &a);
        carry_round_KC(&acc, &acc_d);
        reduce(&acc, &acc);
    }
    pack_field_elem_KC((baseint_t *)tag_packed,
                       &acc); // transform limb representation of field element
                              // (unpacked) into packed field element
    transform_field_elem(
        out, OUTPUTSIZE, tag_packed,
        BUFFSIZE); // transform from field element to byte missing?
}
