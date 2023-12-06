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

#define CONCAT_LVL 5

// classical horner with reduction after multiplication only
void classical_Horner_PK_1B_Delay_b_5C(unsigned char *out,
                                       const unsigned char *in,
                                       unsigned long long inlen,
                                       const unsigned char *key) {
    if (inlen == 0) {
        memset(out, 0, OUTPUTSIZE);
        return;
    }
    unsigned int i;
    field_elem_t acc[CONCAT_LVL] = {0}; // accumulator for horner algorithm
    dfield_elem_t acc_d[CONCAT_LVL] = {0};
    field_elem_t
        a; // temporary field element representing the block being processed
    field_elem_t k[CONCAT_LVL]; // univariate key
    field_elem_precomputed_t k_p[CONCAT_LVL];
    // unsigned char buff[BUFFSIZE] = {0};
    unsigned char transkey[BUFFSIZE] = {0};
    unsigned char tag_packed[BUFFSIZE] = {0}; // what is this size?
    // #pragma unroll
    for (i = 0; i < CONCAT_LVL; ++i) {
        // Transform key from a byte array to one field elements
        transform_key(transkey, BUFFSIZE, key,
                      KEYSIZE /
                          CONCAT_LVL); // transform key from bytes to a packed
                                       // field elements of BLOCKSIZE bytes
        unpack_field_elem(
            k + i, (baseint_t *)transkey); // transform field element(packed)
                                           // into limb representation
        precompute_factor(k_p + i, k + i);
        key += KEYSIZE / CONCAT_LVL;
    }

    // processing all blocks except the last one (possibly smaller)
    while (inlen > BLOCKSIZE) {
        // transform_msg(buff, BUFFSIZE, in, BLOCKSIZE); // transform msg block
        // from bytes to field elements (packed) unpack_field_elem(&a,
        // (baseint_t *) buff); // transform field element(packed) into limb
        // representation
        unpack_and_encode_field_elem(&a, (baseint_t *)in);
        // #pragma unroll
        for (i = 0; i < CONCAT_LVL; ++i) {
            field_add(acc + i, acc + i, &a);
            field_mul_precomputed_no_carry(acc_d + i, acc + i, k_p + i);
            carry_round(acc + i, acc_d + i);
        }
        inlen -= BLOCKSIZE;
        in += BLOCKSIZE;
    }
    // processing last block
    if (inlen) {
        // memset(buff, 0, BUFFSIZE);
        // transform_msg(buff, BUFFSIZE, in, inlen); // transform msg block from
        // bytes to field elements (packed) unpack_field_elem(&a, (baseint_t *)
        // buff); // transform field element(packed) into limb representation
        unpack_and_encode_last_field_elem(&a, (baseint_t *)in, inlen);
        // #pragma unroll
        for (i = 0; i < CONCAT_LVL; ++i) {
            field_add(acc + i, acc + i, &a);
            _carry_round(acc + i, acc + i);
            reduce(acc + i, acc + i);
        }
    }
    // #pragma unroll
    for (i = 0; i < CONCAT_LVL; ++i) {
        pack_field_elem((baseint_t *)tag_packed, acc + i);
        transform_field_elem(out, OUTPUTSIZE / CONCAT_LVL, tag_packed,
                             BUFFSIZE);
        out += OUTPUTSIZE / CONCAT_LVL;
    }
}
