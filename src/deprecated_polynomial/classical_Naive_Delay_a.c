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

// reduction only at the end after addition
void classical_Naive_Delay_a(unsigned char *out, const unsigned char *in,
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
    dfield_elem_t acc_d, a_d;
    field_elem_t
        a; // temporary field element representing the block being processed
    field_elem_t k[2]; // univariate key
    // unsigned char buff[BUFFSIZE] = {0};
    unsigned char transkey[BUFFSIZE] = {0};
    unsigned char tag_packed[BUFFSIZE] = {0}; // what is this size?

    // Transform key from a byte array to one field elements
    transform_key(transkey, BUFFSIZE, key,
                  KEYSIZE); // transform key from bytes to a packed field
                            // elements of BLOCKSIZE bytes
    unpack_field_elem(k,
                      (baseint_t *)transkey); // transform field element(packed)
                                              // into limb representation

    in_temp = in + (noOfBlocks - 1) * BLOCKSIZE;
    // process last block
    // transform_msg(buff, BUFFSIZE, in_temp, inlen-(noOfBlocks-1)*BLOCKSIZE);
    // // transform msg block from bytes to field elements (packed)
    // unpack_field_elem(&acc, (baseint_t *) buff); // transform field
    // element(packed) into limb representation
    unpack_and_encode_last_field_elem(&acc, (baseint_t *)in_temp,
                                      inlen - (noOfBlocks - 1) * BLOCKSIZE);

    // process second last block
    if (noOfBlocks > 1) {
        in_temp -= BLOCKSIZE;
        // transform_msg(buff, BUFFSIZE, in_temp, BLOCKSIZE);
        // unpack_field_elem(&a, (baseint_t *) buff);
        unpack_and_encode_field_elem(&a, (baseint_t *)in_temp);
        field_mul_no_carry(&a_d, &a, k);
        field_add_mix(&acc_d, &a_d, &acc);

        // process the rest of the msg
        if (noOfBlocks > 2) {
            in_temp -= BLOCKSIZE;
            // transform_msg(buff, BUFFSIZE, in_temp, BLOCKSIZE); // transform
            // msg block from bytes to field elements (packed)
            // unpack_field_elem(&a, (baseint_t *) buff); // transform field
            // element(packed) into limb representation
            unpack_and_encode_field_elem(&a, (baseint_t *)in_temp);
            field_mul(k + 1, k, k);
            field_mul_no_carry(&a_d, &a, k + 1);
            field_add_dbl(&acc_d, &acc_d, &a_d);
            while (in_temp != in) {
                in_temp -= BLOCKSIZE;
                // transform_msg(buff, BUFFSIZE, in_temp, BLOCKSIZE); //
                // transform msg block from bytes to field elements (packed)
                // unpack_field_elem(&a, (baseint_t *) buff); // transform field
                // element(packed) into limb representation
                unpack_and_encode_field_elem(&a, (baseint_t *)in_temp);
                field_mul(k + 1, k + 1, k);
                field_mul_no_carry(&a_d, &a, k + 1);
                field_add_dbl(&acc_d, &acc_d, &a_d);
            }
        }
        carry_round(&acc, &acc_d);
    }
    reduce(&acc, &acc);
    pack_field_elem((baseint_t *)tag_packed,
                    &acc); // transform limb representation of field element
                           // (unpacked) into packed field element
    transform_field_elem(
        out, OUTPUTSIZE, tag_packed,
        BUFFSIZE); // transform from field element to byte missing?
}
