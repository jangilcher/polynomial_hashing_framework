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

// classical horner with reduction after multiplication only
void classical_Horner_PK_1B_Delay_b_2C_bis(unsigned char *out,
                                           const unsigned char *in,
                                           unsigned long long inlen,
                                           const unsigned char *key) {
    if (inlen == 0) {
        memset(out, 0, OUTPUTSIZE);
        return;
    }
    field_elem_t acc[2] = {0}; // accumulator for horner algorithm
    dfield_elem_t acc_d[2] = {0};
    field_elem_t
        a; // temporary field element representing the block being processed
    field_elem_t k[2]; // univariate key
    field_elem_precomputed_t k_p[2];
    // unsigned char buff[BUFFSIZE] = {0};
    unsigned char transkey[BUFFSIZE] = {0};
    unsigned char tag_packed[BUFFSIZE] = {0}; // what is this size?

    transform_key(transkey, BUFFSIZE, key,
                  KEYSIZE / 2); // transform key from bytes to a packed field
                                // elements of BLOCKSIZE bytes
    unpack_field_elem(k,
                      (baseint_t *)transkey); // transform field element(packed)
                                              // into limb representation
    precompute_factor(k_p, k);
    transform_key(transkey, BUFFSIZE, key + KEYSIZE / 2,
                  KEYSIZE / 2); // transform key from bytes to a packed field
                                // elements of BLOCKSIZE bytes
    unpack_field_elem(k + 1,
                      (baseint_t *)transkey); // transform field element(packed)
                                              // into limb representation
    precompute_factor(k_p + 1, k + 1);

    // processing all blocks except the last one (possibly smaller)
    while (inlen > BLOCKSIZE) {
        // transform_msg(buff, BUFFSIZE, in, BLOCKSIZE); // transform msg block
        // from bytes to field elements (packed) unpack_field_elem(&a,
        // (baseint_t *) buff); // transform field element(packed) into limb
        // representation
        unpack_and_encode_field_elem(&a, (baseint_t *)in);
        field_add(acc, acc, &a);
        field_add(acc + 1, acc + 1, &a);
        field_mul_precomputed_no_carry(acc_d, acc, k_p);
        field_mul_precomputed_no_carry(acc_d + 1, acc + 1, k_p + 1);
        carry_round(acc, acc_d);
        carry_round(acc + 1, acc_d + 1);
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
        field_add(acc, acc, &a);
        field_add(acc + 1, acc + 1, &a);
        _carry_round(acc, acc);
        reduce(acc, acc);
        _carry_round(acc + 1, acc + 1);
        reduce(acc + 1, acc + 1);
    }
    pack_field_elem((baseint_t *)tag_packed, acc);
    transform_field_elem(out, OUTPUTSIZE / 2, tag_packed, BUFFSIZE);
    pack_field_elem((baseint_t *)tag_packed, acc + 1);
    transform_field_elem(out + OUTPUTSIZE / 2, OUTPUTSIZE / 2, tag_packed,
                         BUFFSIZE);
}
