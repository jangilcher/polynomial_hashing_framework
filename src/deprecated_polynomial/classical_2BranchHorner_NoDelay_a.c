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

void classical_2BranchHorner_NoDelay_a(unsigned char *out,
                                       const unsigned char *in,
                                       unsigned long long inlen,
                                       const unsigned char *key) {
    if (inlen == 0) {
        memset(out, 0, OUTPUTSIZE);
        return;
    }
    field_elem_t acc[2] = {
        0}; // accumulator for horner algorithm -> todo: check initialization
    field_elem_t
        a[2]; // temporary field element representing the block being processed
    field_elem_t k[2];
    // unsigned char buff[2][BUFFSIZE] = {0};
    unsigned char transkey[BUFFSIZE] = {0};
    unsigned char tag_packed[BUFFSIZE] = {0};
    unsigned long long double_blocksize = BLOCKSIZE + BLOCKSIZE;

    // Transform key from a byte array to one field elements
    transform_key(transkey, BUFFSIZE, key,
                  KEYSIZE); // transform key from bytes to a packed field
                            // elements of BLOCKSIZE bytes
    unpack_field_elem(k,
                      (baseint_t *)transkey); // transform field element(packed)
                                              // into limb representation

    // process msg of only 1 block
    if (inlen <= BLOCKSIZE) {
        // transform_msg(tag_packed, BUFFSIZE, in, inlen); // transform msg
        // block from bytes to field elements (packed)
        unpack_and_encode_last_field_elem(
            acc, (baseint_t *)in,
            inlen); // Not optimized, unpack then pack is redundant, only need
                    // encode/transform
        pack_field_elem((baseint_t *)tag_packed,
                        acc); // Not optimized, unpack then pack is redundant,
                              // only need encode/transform
    } else if (inlen <= double_blocksize) { // process msg of only 2 blocks
        // transform_msg(buff[0], BUFFSIZE, in, BLOCKSIZE); // transform msg
        // block from bytes to field elements (packed) unpack_field_elem(a,
        // (baseint_t *) buff[0]); // transform field element(packed) into limb
        // representation
        unpack_and_encode_field_elem(a, (baseint_t *)in);
        in += BLOCKSIZE;
        inlen -= BLOCKSIZE;
        // transform_msg(buff[1], BUFFSIZE, in, inlen);
        // unpack_field_elem(a+1, (baseint_t *) buff[1]);
        unpack_and_encode_last_field_elem(a + 1, (baseint_t *)in, inlen);

        field_mul_reduce(a, a, k);
        field_add_reduce(acc, a, a + 1);
        pack_field_elem((baseint_t *)tag_packed,
                        acc); // transform limb representation of field element
                              // (unpacked) into packed field element
    } else {                  // process msg of more than 2 Blocks
        field_mul_reduce(k + 1, k, k); // square of key for 2BranchHorner

        // initialize accumulators
        // transform_msg(buff[0], BUFFSIZE, in, BLOCKSIZE);
        // unpack_field_elem(acc, (baseint_t *) buff[0]);
        unpack_and_encode_field_elem(acc, (baseint_t *)in);
        in += BLOCKSIZE;
        // transform_msg(buff[1], BUFFSIZE, in, BLOCKSIZE);
        // unpack_field_elem(acc+1, (baseint_t *) buff[1]);
        unpack_and_encode_field_elem(acc + 1, (baseint_t *)in);
        in += BLOCKSIZE;
        inlen -= double_blocksize;

        while (inlen > double_blocksize) {
            // transform_msg(buff[0], BUFFSIZE, in, BLOCKSIZE); // transform msg
            // block from bytes to field elements (packed) unpack_field_elem(a,
            // (baseint_t *) buff[0]); // transform field element(packed) into
            // limb representation
            unpack_and_encode_field_elem(a, (baseint_t *)in);
            in += BLOCKSIZE;
            // transform_msg(buff[1], BUFFSIZE, in, BLOCKSIZE);
            // unpack_field_elem(a+1, (baseint_t *) buff[1]);
            unpack_and_encode_field_elem(a + 1, (baseint_t *)in);
            in += BLOCKSIZE;
            inlen -= double_blocksize;

            field_mul_reduce(acc, acc, k + 1);
            field_mul_reduce(acc + 1, acc + 1, k + 1);
            field_add_reduce(acc, acc, a);
            field_add_reduce(acc + 1, acc + 1, a + 1);
        }

        // process msg of 2 blocks with last one possibly incomplete
        if (inlen > BLOCKSIZE) {
            // transform_msg(buff[0], BUFFSIZE, in, BLOCKSIZE); // transform msg
            // block from bytes to field elements (packed) unpack_field_elem(a,
            // (baseint_t *) buff[0]); // transform field element(packed) into
            // limb representation
            unpack_and_encode_field_elem(a, (baseint_t *)in);
            in += BLOCKSIZE;
            inlen -= BLOCKSIZE;
            // transform_msg(buff[1], BUFFSIZE, in, inlen);
            // unpack_field_elem(a+1, (baseint_t *) buff[1]);
            unpack_and_encode_last_field_elem(a + 1, (baseint_t *)in, inlen);

            field_mul_reduce(acc, acc, k + 1);
            field_mul_reduce(acc + 1, acc + 1, k + 1);
            field_add_reduce(acc, acc, a);
            field_add_reduce(acc + 1, acc + 1, a + 1);

            // reducing to only one accumulator
            field_mul_reduce(acc, acc, k);
            field_add_reduce(acc, acc, acc + 1);
        } else { // processing last msg block
            // transform_msg(buff[0], BUFFSIZE, in, inlen); // transform msg
            // block from bytes to field elements (packed) unpack_field_elem(a,
            // (baseint_t *) buff[0]); // transform field element(packed) into
            // limb representation
            unpack_and_encode_last_field_elem(a, (baseint_t *)in, inlen);
            field_mul_reduce(acc, acc, k + 1);
            field_add_reduce(acc, acc, a);

            // reducing to only one accumulator
            field_mul_reduce(acc + 1, acc + 1, k);
            field_add_reduce(acc, acc, acc + 1);
        }
        pack_field_elem((baseint_t *)tag_packed,
                        acc); // transform limb representation of field element
                              // (unpacked) into packed field element
    }
    transform_field_elem(
        out, OUTPUTSIZE, tag_packed,
        BUFFSIZE); // transform from field element to byte missing?
}
