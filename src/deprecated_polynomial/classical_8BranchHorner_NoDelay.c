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

void classical_8BranchHorner_NoDelay(unsigned char *out,
                                     const unsigned char *in,
                                     unsigned long long inlen,
                                     const unsigned char *key) {
    if (inlen == 0) {
        memset(out, 0, OUTPUTSIZE);
        return;
    }
    unsigned int nbBranch = 8;
    unsigned int i, j;
    field_elem_t acc[nbBranch]; // accumulator for horner algorithm
    field_elem_t a[nbBranch]; // temporary field element representing the block
                              // being processed
    field_elem_t k[nbBranch];
    // unsigned char buff[nbBranch][BUFFSIZE];
    unsigned char transkey[BUFFSIZE] = {0};
    unsigned char tag_packed[BUFFSIZE] = {0};
    unsigned long long nbBranch_blocksize = nbBranch * BLOCKSIZE;

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
    } else if (inlen <=
               nbBranch_blocksize) { // process msg of nbBranch Blocks or less
        for (i = 2; i < nbBranch + 1; ++i) {
            if (inlen <= i * BLOCKSIZE) {
                // compute key powers
                for (j = 0; j < i - 2; ++j) {
                    field_mul_reduce(k + j + 1, k + j, k);
                }
                for (j = 0; j < i - 1; ++j) {
                    // transform_msg(buff[j], BUFFSIZE, in, BLOCKSIZE); //
                    // transform msg block from bytes to field elements (packed)
                    // unpack_field_elem(a+j, (baseint_t *) buff[j]); //
                    // transform field element(packed) into limb representation
                    unpack_and_encode_field_elem(a + j, (baseint_t *)in);
                    in += BLOCKSIZE;
                    inlen -= BLOCKSIZE;
                }
                // transform_msg(buff[i-1], BUFFSIZE, in, inlen);
                // unpack_field_elem(a+i-1, (baseint_t *) buff[i-1]);
                unpack_and_encode_last_field_elem(a + i - 1, (baseint_t *)in,
                                                  inlen);

                field_mul_reduce(a, a, k + i - 2);
                field_add_reduce(acc, a, a + i - 1);
                for (j = 1; j < i - 1; ++j) {
                    field_mul_reduce(a + j, a + j, k + i - 2 - j);
                    field_add_reduce(acc, acc, a + j);
                }
                pack_field_elem((baseint_t *)tag_packed, acc);
                break;
            }
        }
    } else { // process msg of more than nbBranch Blocks
        // if (inlen > nbBranch_blocksize) {
        // compute key powers for parallel horner
        for (i = 0; i < nbBranch - 1; ++i) {
            field_mul_reduce(k + i + 1, k + i, k);
        }
        // initialize accumulators
        for (i = 0; i < nbBranch; ++i) {
            // transform_msg(buff[i], BUFFSIZE, in, BLOCKSIZE);
            // unpack_field_elem(acc+i, (baseint_t *) buff[i]);
            unpack_and_encode_field_elem(acc + i, (baseint_t *)in);
            in += BLOCKSIZE;
        }
        inlen -= nbBranch_blocksize;

        while (inlen > nbBranch_blocksize) {
            for (i = 0; i < nbBranch; ++i) {
                // transform_msg(buff[i], BUFFSIZE, in, BLOCKSIZE);
                // unpack_field_elem(a+i, (baseint_t *) buff[i]);
                unpack_and_encode_field_elem(a + i, (baseint_t *)in);
                in += BLOCKSIZE;
            }
            inlen -= nbBranch_blocksize;

            for (i = 0; i < nbBranch; ++i) {
                field_mul_reduce(acc + i, acc + i, k + nbBranch - 1);
                field_add_reduce(acc + i, acc + i, a + i);
            }
        }
        // process remaining msg of length less or equal to nbBranch Blocks
        for (i = 0; i < nbBranch; ++i) {
            if (inlen > (nbBranch - i - 1) * BLOCKSIZE) {
                for (j = 0; j < nbBranch - i - 1; ++j) {
                    // transform_msg(buff[j], BUFFSIZE, in, BLOCKSIZE); //
                    // transform msg block from bytes to field elements (packed)
                    // unpack_field_elem(a+j, (baseint_t *) buff[j]); //
                    // transform field element(packed) into limb representation
                    unpack_and_encode_field_elem(a + j, (baseint_t *)in);
                    in += BLOCKSIZE;
                    inlen -= BLOCKSIZE;
                }
                // transform_msg(buff[nbBranch-i-1], BUFFSIZE, in, inlen);
                // unpack_field_elem(a+nbBranch-i-1, (baseint_t *)
                // buff[nbBranch-i-1]);
                unpack_and_encode_last_field_elem(a + nbBranch - i - 1,
                                                  (baseint_t *)in, inlen);
                // inlen=0;
                for (j = 0; j < nbBranch - i; ++j) {
                    field_mul_reduce(acc + j, acc + j, k + nbBranch - 1);
                    field_add_reduce(acc + j, acc + j, a + j);
                }

                // reducing to only one accumulator
                for (j = nbBranch - i; j < nbBranch; ++j) {
                    field_mul_reduce(acc + j, acc + j,
                                     k + nbBranch + nbBranch - j - i - 2);
                }
                for (j = 0; j < nbBranch - i - 1; ++j) {
                    field_mul_reduce(acc + j, acc + j,
                                     k + nbBranch - j - i - 2);
                }
                for (j = 1; j < nbBranch; ++j) {
                    field_add_reduce(acc, acc, acc + j);
                }
                break;
            }
        }
        pack_field_elem((baseint_t *)tag_packed,
                        acc); // transform limb representation of field element
                              // (unpacked) into packed field element
    }
    transform_field_elem(
        out, OUTPUTSIZE, tag_packed,
        BUFFSIZE); // transform from field element to byte missing?
}
