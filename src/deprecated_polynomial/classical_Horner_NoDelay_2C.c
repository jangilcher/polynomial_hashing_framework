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

#define CONCAT_LVL 2

void classical_Horner_NoDelay_2C(unsigned char *out, const unsigned char *in,
                                 unsigned long long inlen,
                                 const unsigned char *key) {
    if (inlen == 0) {
        memset(out, 0, OUTPUTSIZE);
        return;
    }
    unsigned int i;
    field_elem_t acc[CONCAT_LVL] = {0}; // accumulator for horner algorithm
    field_elem_t
        a; // temporary field element representing the block being processed
    field_elem_t k[CONCAT_LVL]; // univariate key
    // unsigned char buff[BUFFSIZE] = {0};
    unsigned char transkey[BUFFSIZE] = {0};
    unsigned char tag_packed[BUFFSIZE] = {0};

    for (i = 0; i < CONCAT_LVL; ++i) {
        // Transform key from a byte array to one field elements
        transform_key(transkey, BUFFSIZE, key,
                      KEYSIZE /
                          CONCAT_LVL); // transform key from bytes to a packed
                                       // field elements of BLOCKSIZE bytes
        unpack_field_elem(
            k + i, (baseint_t *)transkey); // transform field element(packed)
                                           // into limb representation
        key += KEYSIZE / CONCAT_LVL;
    }

    // processing all blocks except the last one (possibly smaller)
    while (inlen > BLOCKSIZE) {
        unpack_and_encode_field_elem(&a, (baseint_t *)in);
        for (i = 0; i < CONCAT_LVL; ++i) {
            field_add_reduce(acc + i, acc + i, &a);
            field_mul_reduce(acc + i, acc + i, k + i);
        }
        inlen -= BLOCKSIZE;
        in += BLOCKSIZE;
    }
    // processing last block
    if (inlen) {
        unpack_and_encode_last_field_elem(&a, (baseint_t *)in, inlen);
        for (i = 0; i < CONCAT_LVL; ++i) {
            field_add_reduce(acc + i, acc + i, &a);
        }
    }
    for (i = 0; i < CONCAT_LVL; ++i) {
        pack_field_elem((baseint_t *)tag_packed, acc + i);
        transform_field_elem(out, OUTPUTSIZE / CONCAT_LVL, tag_packed,
                             BUFFSIZE);
        out += OUTPUTSIZE / CONCAT_LVL;
    }
}
