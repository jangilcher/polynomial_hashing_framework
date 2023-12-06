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

void DCHM_NoDelay(unsigned char *out, const unsigned char *in,
                  unsigned long long inlen, const unsigned char *key) {
    if (inlen == 0) {
        memset(out, 0, OUTPUTSIZE);
        return;
    }
    // unsigned long long noOfBlocks = inlen/BLOCKSIZE + (inlen % BLOCKSIZE !=
    // 0);
    field_elem_t acc = {0};
    field_elem_t a[2];
    field_elem_t k[2];
    field_elem_t ks;
    unsigned char buff[2][BUFFSIZE] = {0};
    unsigned char transkey[BUFFSIZE] = {0};
    unsigned char tag_packed[BUFFSIZE] = {0};
    unsigned long long double_blocksize = 2 * BLOCKSIZE;

    transform_key(transkey, BUFFSIZE, key, KEYSIZE);
    unpack_field_elem(k, (baseint_t *)transkey);

    if (inlen > BLOCKSIZE) {
        field_sqr_reduce(k + 1, k);
        memcpy(&ks, k + 1, sizeof(field_elem_t));
    }

    // Process blocks by 2
    while (inlen > double_blocksize) {
        transform_msg(buff[0], BUFFSIZE, in, BLOCKSIZE);
        unpack_field_elem(a, (baseint_t *)buff[0]);
        in += BLOCKSIZE;
        transform_msg(buff[1], BUFFSIZE, in, BLOCKSIZE);
        unpack_field_elem(a + 1, (baseint_t *)buff[1]);
        in += BLOCKSIZE;
        inlen -= double_blocksize;

        field_add_reduce(a, a, k);
        field_add_reduce(a + 1, a + 1, k + 1);
        field_mul_reduce(a, a, a + 1);
        field_add_reduce(&acc, &acc, a);

        field_mul_reduce(k, k, &ks);
        field_mul_reduce(k + 1, k + 1, &ks);
    }

    // Process last 2 blocks
    if (inlen > BLOCKSIZE) {
        transform_msg(buff[0], BUFFSIZE, in, BLOCKSIZE);
        unpack_field_elem(a, (baseint_t *)buff[0]);
        in += BLOCKSIZE;
        inlen -= BLOCKSIZE;
        transform_msg(buff[1], BUFFSIZE, in, inlen);
        unpack_field_elem(a + 1, (baseint_t *)buff[1]);
        in += BLOCKSIZE;

        field_add_reduce(a, a, k);
        field_add_reduce(a + 1, a + 1, k + 1);
        field_mul_reduce(a, a, a + 1);
        field_add_reduce(&acc, &acc, a);
    } else {
        transform_msg(buff[0], BUFFSIZE, in, inlen);
        unpack_field_elem(a, (baseint_t *)buff[0]);
        field_add_reduce(&acc, &acc, a);
    }

    pack_field_elem((baseint_t *)tag_packed, &acc);
    transform_field_elem(out, OUTPUTSIZE, tag_packed, BUFFSIZE);
}
