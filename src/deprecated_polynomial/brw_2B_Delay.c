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

#define LOG2(X)                                                                \
    ((unsigned)(8 * sizeof(unsigned long long) - __builtin_clzll((X)) - 1))

// do reduction after every multiplication
void brw_2B_Delay(unsigned char *out, const unsigned char *in,
                  unsigned long long inlen, const unsigned char *key) {
    if (inlen == 0) {
        memset(out, 0, OUTPUTSIZE);
        return;
    }
    unsigned long long blkctr = 0;
    field_elem_t
        a[4]; // temporary field element representing the block being processed

    unsigned long long noOfBlocks =
        inlen / BLOCKSIZE + (inlen % BLOCKSIZE != 0);
    // printf("start func1 inlen= %llu, noOfBlocks= %llu\n", inlen, noOfBlocks);

    unsigned int lg = LOG2(noOfBlocks), i, sp; // log(noOfBlocks)/log(2);
    // printf("start func2 lg= %d\n",lg);
    field_elem_t k[lg + 1]; // powers of univariate key
    field_elem_t T[lg];
    unsigned char transkey[BUFFSIZE] = {0};
    unsigned char tag_packed[BUFFSIZE] = {0};

    // Transform key from a byte array to one field elements
    transform_key(transkey, BUFFSIZE, key,
                  KEYSIZE); // transform key from bytes to a packed field
                            // elements of BLOCKSIZE bytes
    unpack_field_elem(k,
                      (baseint_t *)transkey); // transform field element(packed)
                                              // into limb representation
    if (noOfBlocks > 2)
        for (i = 0; i < lg; ++i) {
            field_sqr(k + i + 1, k + i);
        }

    while (inlen > blkctr + 4 * BLOCKSIZE) {
        // printf("start loop ctr= %llu \n",blkctr);
        //  unpack 4 Blocks
        for (i = 0; i < 4; ++i) {
            unpack_and_encode_field_elem(a + i, (baseint_t *)(in + blkctr));
            blkctr += BLOCKSIZE;
        }
        field_add(a, a, k);
        field_add(a + 1, a + 1, k + 1);
        field_mul(a, a, a + 1);
        field_add(a, a, a + 2);

        sp = __builtin_ctz(blkctr) - __builtin_ctz(BLOCKSIZE); // s+2
        // s = __builtin_ctz(blkctr)-2-LOG2(BLOCKSIZE);// TODO: fix for
        // blockcounter not a power of 2 printf("Starting sum \n");
        for (i = 0; i + 2 < sp; ++i) {
            field_add(a, a, T + i);
        }
        // printf("End sum i= %llu \n", i);
        field_add(a + 3, a + 3, k + sp);
        field_mul(T + sp - 2, a, a + 3);
    }
    // printf("finish loop \n");
    switch (noOfBlocks % 4) {
    case 0:
        for (i = 0; i < 3; ++i) {
            unpack_and_encode_field_elem(a + i, (baseint_t *)(in + blkctr));
            blkctr += BLOCKSIZE;
        }
        unpack_and_encode_last_field_elem(a + 3, (baseint_t *)(in + blkctr),
                                          ((inlen - 1) % BLOCKSIZE) + 1);
        blkctr += BLOCKSIZE;
        field_add(a, a, k);
        field_add(a + 1, a + 1, k + 1);
        field_mul(a, a, a + 1);
        field_add(a, a, a + 2);

        sp = __builtin_ctz(blkctr) - __builtin_ctz(BLOCKSIZE); // s+2
        for (i = 0; i + 2 < sp; ++i) {
            field_add(a, a, T + i);
        }
        // printf("End sum i= %llu \n", i);
        field_add(a + 3, a + 3, k + sp);
        field_mul(T + sp - 2, a, a + 3);
        // a->val[0] = 0;
        // a->val[1] = 0;
        // a->val[2] = 0;
        a[0] = (field_elem_t){0};
        break;
    case 1:
        unpack_and_encode_last_field_elem(a, (baseint_t *)(in + blkctr),
                                          ((inlen - 1) % BLOCKSIZE) + 1);
        break;

    case 2:
        unpack_and_encode_field_elem(a, (baseint_t *)(in + blkctr));
        blkctr += BLOCKSIZE;
        unpack_and_encode_last_field_elem(a + 1, (baseint_t *)(in + blkctr),
                                          ((inlen - 1) % BLOCKSIZE) + 1);

        field_mul(a, a, k);
        field_add(a, a, a + 1);
        break;
    case 3:
        for (i = 0; i < 2; ++i) {
            unpack_and_encode_field_elem(a + i, (baseint_t *)(in + blkctr));
            blkctr += BLOCKSIZE;
        }
        unpack_and_encode_last_field_elem(a + 2, (baseint_t *)(in + blkctr),
                                          ((inlen - 1) % BLOCKSIZE) + 1);
        field_add(a, a, k);
        field_add(a + 1, a + 1, k + 1);
        field_mul(a, a, a + 1);
        field_add(a, a, a + 2);
        break;
    }

    noOfBlocks >>= 2;
    for (i = 0; i + 1 < lg; ++i, noOfBlocks >>= 1) {
        if ((noOfBlocks & 1) == 1) {
            field_add(a, a, T + i);
        }
    }

    _carry_round(a, a);
    reduce(a, a);
    pack_field_elem((baseint_t *)tag_packed,
                    a); // transform limb representation of field element
                        // (unpacked) into packed field element
    transform_field_elem(
        out, OUTPUTSIZE, tag_packed,
        BUFFSIZE); // transform from field element to byte missing?
}
