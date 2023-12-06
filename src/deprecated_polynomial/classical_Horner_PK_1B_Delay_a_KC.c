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

#if OUTER_PARAM0 == 64
#include "../polynomial/pf_arithmetic_key_clamping.h"
#elif OUTER_PARAM0 == 32
#include "../polynomial/pf_arithmetic_key_clamping_32.h"
#endif

// classical horner with reduction after addition only
// with field arithmetic that uses a mixed of low and upper bit key clamping as
// poly1305
void classical_Horner_PK_1B_Delay_a_KC(unsigned char *out,
                                       const unsigned char *in,
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
    field_elem_precomputed_t k_p;
    unsigned char transkey[BUFFSIZE] = {0};
    unsigned char tag_packed[BUFFSIZE] = {0};

    // Transform key from a byte array to one field elements
    transform_key(transkey, BUFFSIZE, key, KEYSIZE);
    unpack_field_elem_KC(&k, (baseint_t *)transkey);
    precompute_factor_KC(&k_p, &k);

    // processing all blocks except the last one (possibly smaller)
    while (inlen > BLOCKSIZE) {
        unpack_and_encode_field_elem_KC(&a, (baseint_t *)in);
        field_add_mix(&acc_d, &acc_d, &a);
        carry_round_KC(&acc, &acc_d);
        field_mul_precomputed_no_carry_KC(&acc_d, &acc, &k_p);
        inlen -= BLOCKSIZE;
        in += BLOCKSIZE;
    }
    // processing last block
    if (inlen) {
        unpack_and_encode_last_field_elem_KC(&a, (baseint_t *)in, inlen);
        field_add_mix(&acc_d, &acc_d, &a);
        carry_round_KC_final(&acc, &acc_d);
        reduce_KC(&acc, &acc);
    }
    pack_field_elem_KC((baseint_t *)tag_packed,
                       &acc); // transform limb representation of field element
                              // (unpacked) into packed field element
    transform_field_elem(
        out, OUTPUTSIZE, tag_packed,
        BUFFSIZE); // transform from field element to byte missing?
}
