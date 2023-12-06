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
unpack_field_elem_bis(field_elem_t *res, uint64_t *a) {
    res->val[0] = a[0];
    res->val[1] = a[1];
}

inline __attribute__((always_inline)) void
field_mul_Tradix(field_elem_t *res, field_elem_t *a, field_elem_t *b) {
    // uint128_t c;
    uint128_t acc;
    uint128_t d[3] = {0};

    acc = ((uint128_t)a->val[0] * b->val[0]);
    d[0] += acc;
    acc = ((uint128_t)a->val[1] * b->val[0]);
    d[1] += acc;
    acc = ((uint128_t)a->val[2] * b->val[0]);
    d[2] += acc;

    acc = ((uint128_t)a->val[0] * b->val[1]);
    d[1] += (((uint64_t)acc) & (((((uint64_t)1) << 24) - 1))) << 20;
    d[2] += (uint128_t)(acc >> 24);

    acc = ((uint128_t)a->val[1] * b->val[1]);
    d[2] += (((uint64_t)acc) & (((((uint64_t)1) << 22) - 1))) << 20;
    d[0] += ((uint128_t)(acc >> 22) * 5);

    acc = ((uint128_t)a->val[2] * b->val[1] * 5);
    d[0] += (((uint64_t)acc) & (((((uint64_t)1) << 22) - 1))) << 22;
    d[1] += (uint128_t)(acc >> 22);

    acc = (uint128_t)(d[0] >> (44));
    res->val[0] = ((uint64_t)(d[0])) & (((((uint64_t)1) << 44) - 1));
    d[1] += acc;
    acc = (uint128_t)(d[1] >> (44));
    res->val[1] = ((uint64_t)(d[1])) & (((((uint64_t)1) << 44) - 1));
    d[2] += acc;
    acc = (uint128_t)(d[2] >> (42));
    res->val[2] = ((uint64_t)(d[2])) & (((((uint64_t)1) << 42) - 1));
    // res->val[0] += acc * 5;
    acc = (uint128_t)(res->val[0] + acc * 5);
    res->val[0] = ((uint64_t)(acc)) & (((((uint64_t)1) << 44) - 1));
    // c = (uint128_t) (res->val[0] >> (44));
    res->val[1] += (uint64_t)(acc >> (44));
}

inline __attribute__((always_inline)) void
field_mul_reduce_Tradix(field_elem_t *res, field_elem_t *a, field_elem_t *b) {
    field_elem_t tmp;
    field_mul_Tradix(&tmp, a, b);
    reduce(res, &tmp);
}

void classical_Naive_NoDelay_2radix(unsigned char *out, const unsigned char *in,
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
    field_elem_t
        a; // temporary field element representing the block being processed
    field_elem_t k[2]; // univariate key
    unsigned char buff[BUFFSIZE] = {0};
    unsigned char transkey[BUFFSIZE] = {0};
    unsigned char tag_packed[BUFFSIZE] = {0};

    // Transform key from a byte array to one field elements
    transform_key(transkey, BUFFSIZE, key,
                  KEYSIZE); // transform key from bytes to a packed field
                            // elements of BLOCKSIZE bytes
    unpack_field_elem(k,
                      (baseint_t *)transkey); // transform field element(packed)
                                              // into limb representation

    in_temp = in + (noOfBlocks - 1) * BLOCKSIZE;
    // process last block
    transform_msg(buff, BUFFSIZE, in_temp,
                  inlen - (noOfBlocks - 1) *
                              BLOCKSIZE); // transform msg block from bytes to
                                          // field elements (packed)
    unpack_field_elem(
        &acc,
        (baseint_t *)
            buff); // transform field element(packed) into limb representation

    // process second last block
    if (noOfBlocks > 1) {
        in_temp -= BLOCKSIZE;
        transform_msg(buff, BUFFSIZE, in_temp, BLOCKSIZE);
        unpack_field_elem_bis(&a, (baseint_t *)buff);
        field_mul_reduce_Tradix(&a, k, &a);
        field_add_reduce(&acc, &acc, &a);
    }

    // process the rest of the msg
    if (noOfBlocks > 2) {
        in_temp -= BLOCKSIZE;
        transform_msg(buff, BUFFSIZE, in_temp,
                      BLOCKSIZE); // transform msg block from bytes to field
                                  // elements (packed)
        unpack_field_elem_bis(
            &a, (baseint_t *)buff); // transform field element(packed) into limb
                                    // representation
        field_mul_reduce(k + 1, k, k);
        field_mul_reduce_Tradix(&a, k + 1, &a);
        field_add_reduce(&acc, &acc, &a);
        while (in_temp != in) {
            in_temp -= BLOCKSIZE;
            transform_msg(buff, BUFFSIZE, in_temp,
                          BLOCKSIZE); // transform msg block from bytes to field
                                      // elements (packed)
            unpack_field_elem_bis(
                &a, (baseint_t *)buff); // transform field element(packed) into
                                        // limb representation
            field_mul_reduce(k + 1, k + 1, k);
            field_mul_reduce_Tradix(&a, k + 1, &a);
            field_add_reduce(&acc, &acc, &a);
        }
    }

    pack_field_elem((baseint_t *)tag_packed,
                    &acc); // transform limb representation of field element
                           // (unpacked) into packed field element
    transform_field_elem(
        out, OUTPUTSIZE, tag_packed,
        BUFFSIZE); // transform from field element to byte missing?
}
