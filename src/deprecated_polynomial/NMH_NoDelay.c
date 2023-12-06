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
#include "../key_expansion.h"
#include "../transform/transform.h"
#include <stddef.h>
#include <string.h>

#if defined(INNERPOLY_H) && defined(INNERPOLY)
#include INNERPOLY_H
#endif

void NMH_NoDelay(unsigned char *out, const unsigned char *in,
                 unsigned long long inlen, const unsigned char *key) {
    if (inlen == 0) {
        memset(out, 0, OUTPUTSIZE);
        return;
    }

    unsigned long long blkctr = 0;
    field_elem_t acc = {0};
    field_elem_t a0;
    field_elem_t a1;
    field_elem_t k0;
    field_elem_t k1;
    unsigned char keystream[inlen + BLOCKSIZE];
    unsigned char buff0[BUFFSIZE] = {0};
    unsigned char buff1[BUFFSIZE] = {0};
    unsigned char transkey0[BUFFSIZE] = {0};
    unsigned char transkey1[BUFFSIZE] = {0};
    unsigned char tag_packed[BUFFSIZE] = {0};

    // Generate Keys
    init(key);
    get(keystream,
        inlen +
            BLOCKSIZE); // TODO: adapt for when processing more than BLOCKSIZE
                        // for better security and generate exact number of
                        // randomness needed (here BLOCKSIZE is an upper bound)

    // Process blocks by 2
    while (inlen >= 2 * BLOCKSIZE + blkctr) {
        transform_msg(buff0, BUFFSIZE, in + blkctr, BLOCKSIZE);
        unpack_field_elem(&a0, (baseint_t *)buff0);
        transform_msg(
            transkey0, BUFFSIZE, keystream + blkctr,
            BLOCKSIZE); // TODO: process more than BLOCKSIZE for better security
        unpack_field_elem(&k0, (baseint_t *)transkey0);
        blkctr += BLOCKSIZE;
        transform_msg(buff1, BUFFSIZE, in + blkctr, BLOCKSIZE);
        unpack_field_elem(&a1, (baseint_t *)buff1);
        transform_msg(
            transkey1, BUFFSIZE, keystream + blkctr,
            BLOCKSIZE); // TODO: process more than BLOCKSIZE for better security
        unpack_field_elem(&k1, (baseint_t *)transkey1);
        blkctr += BLOCKSIZE;

        field_add(&a0, &a0, &k0);
        field_add(&a1, &a1, &k1);

        field_mul(&a0, &a0, &a1);
        field_add(&acc, &acc, &a0);
    }

    // Process last two blocks
    if (blkctr + BLOCKSIZE < inlen) {
        transform_msg(buff0, BUFFSIZE, in + blkctr, BLOCKSIZE);
        unpack_field_elem(&a0, (baseint_t *)buff0);
        transform_msg(
            transkey0, BUFFSIZE, keystream + blkctr,
            BLOCKSIZE); // TODO: process more than BLOCKSIZE for better security
        unpack_field_elem(&k0, (baseint_t *)transkey0);
        blkctr += BLOCKSIZE;

        inlen -= blkctr * BLOCKSIZE;
        transform_msg(buff1, BUFFSIZE, in + blkctr, inlen);
        unpack_field_elem(&a1, (baseint_t *)buff1);
        transform_msg(
            transkey1, BUFFSIZE, keystream + blkctr,
            BLOCKSIZE); // TODO: process more than BLOCKSIZE for better security
        unpack_field_elem(&k1, (baseint_t *)transkey1);
        // blkctr += BLOCKSIZE;

        field_add(&a0, &a0, &k0);
        field_add(&a1, &a1, &k1);

        field_mul(&a0, &a0, &a1);
        field_add(&acc, &acc, &a0);
    } else if (blkctr < inlen) { // Process possible last block
        inlen -= blkctr * BLOCKSIZE;
        transform_msg(buff0, BUFFSIZE, in, inlen);
        unpack_field_elem(&a0, (baseint_t *)buff0);
        field_add(&acc, &acc, &a0);
    }

    pack_field_elem((baseint_t *)tag_packed, &acc);
    transform_field_elem(out, OUTPUTSIZE, tag_packed, BUFFSIZE);
}
