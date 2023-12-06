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

// Classical 2 level with reduction after each multiplication
void classical_2level_8B_Delay_c(unsigned char *out, const unsigned char *in,
                                 unsigned long long inlen,
                                 const unsigned char *key) {
    if (inlen == 0) {
        memset(out, 0, OUTPUTSIZE);
        return;
    }
    unsigned int nbBlockFlvl = 8;
    unsigned int i, j;
    field_elem_t acc;
    dfield_elem_t acc_d[nbBlockFlvl];
    field_elem_t a[nbBlockFlvl]; // temporary field element representing the
                                 // block being processed
    field_elem_t k[nbBlockFlvl];
    // unsigned char buff[nbBlockFlvl][BUFFSIZE];
    unsigned char transkey[BUFFSIZE] = {0};
    unsigned char tag_packed[BUFFSIZE] = {0};
    unsigned long long nbBlockFlvl_blocksize = nbBlockFlvl * BLOCKSIZE;

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
        unpack_and_encode_last_field_elem(&acc, (baseint_t *)in, inlen);
        pack_field_elem((baseint_t *)tag_packed, &acc);
    } else if (inlen <= nbBlockFlvl_blocksize) { // process msg of nbBlockFlvl
                                                 // Blocks or less
        for (i = 2; i < nbBlockFlvl + 1; ++i) {
            if (inlen <= i * BLOCKSIZE) {
                // compute key powers
                for (j = 0; j < i - 2; ++j) {
                    field_mul(k + j + 1, k + j, k);
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

                field_mul_no_carry(acc_d, a, k + i - 2);
                carry_round(&acc, acc_d);
                for (j = 1; j < i - 1; ++j) {
                    field_mul_no_carry(acc_d + j, a + j, k + i - 2 - j);
                    carry_round(a + j, acc_d + j);
                    field_add(&acc, &acc, a + j);
                }
                field_add(&acc, &acc, a + i - 1);
                _carry_round(&acc, &acc);
                reduce(&acc, &acc);
                pack_field_elem((baseint_t *)tag_packed, &acc);
                break;
            }
        }
    } else { // process msg of more than nbBlockFlvl Blocks
        // if (inlen > nbBlockFlvl_blocksize) {
        // compute key powers for parallel horner
        for (i = 0; i < nbBlockFlvl - 1; ++i) {
            field_mul_no_carry(acc_d, k + i, k);
            carry_round(k + i + 1, acc_d);
        }
        // initialize accumulator
        for (i = 0; i < nbBlockFlvl; ++i) {
            // transform_msg(buff[i], BUFFSIZE, in, BLOCKSIZE);
            // unpack_field_elem(a+i, (baseint_t *) buff[i]);
            unpack_and_encode_field_elem(a + i, (baseint_t *)in);
            in += BLOCKSIZE;
        }
        inlen -= nbBlockFlvl_blocksize;
        field_mul_no_carry(acc_d, a, k + nbBlockFlvl - 2);
        carry_round(&acc, acc_d);
        for (i = 1; i < nbBlockFlvl - 1; ++i) {
            field_mul_no_carry(acc_d + i, a + i, k + nbBlockFlvl - 2 - i);
            carry_round(a + i, acc_d + i);
            field_add(&acc, &acc, a + i);
        }
        field_add(&acc, &acc, a + nbBlockFlvl - 1);

        while (inlen > nbBlockFlvl_blocksize) {
            for (i = 0; i < nbBlockFlvl; ++i) {
                // transform_msg(buff[i], BUFFSIZE, in, BLOCKSIZE);
                // unpack_field_elem(a+i, (baseint_t *) buff[i]);
                unpack_and_encode_field_elem(a + i, (baseint_t *)in);
                in += BLOCKSIZE;
            }
            inlen -= nbBlockFlvl_blocksize;

            field_mul_no_carry(acc_d, &acc, k + nbBlockFlvl - 1);
            carry_round(&acc, acc_d);
            for (i = 0; i < nbBlockFlvl - 1; ++i) {
                field_mul_no_carry(acc_d + i + 1, a + i,
                                   k + nbBlockFlvl - 2 - i);
                carry_round(a + i, acc_d + i + 1);
                field_add(&acc, &acc, a + i);
            }
            field_add(&acc, &acc, a + nbBlockFlvl - 1);
        }
        // process remaining msg of length less or equal to nbBlockFlvl Blocks
        for (i = 0; i < nbBlockFlvl; ++i) {
            if (inlen > (nbBlockFlvl - i - 1) * BLOCKSIZE) {
                for (j = 0; j < nbBlockFlvl - i - 1; ++j) {
                    // transform_msg(buff[j], BUFFSIZE, in, BLOCKSIZE); //
                    // transform msg block from bytes to field elements (packed)
                    // unpack_field_elem(a+j, (baseint_t *) buff[j]); //
                    // transform field element(packed) into limb representation
                    unpack_and_encode_field_elem(a + j, (baseint_t *)in);
                    in += BLOCKSIZE;
                    inlen -= BLOCKSIZE;
                }
                // transform_msg(buff[nbBlockFlvl-i-1], BUFFSIZE, in, inlen);
                // unpack_field_elem(a+nbBlockFlvl-i-1, (baseint_t *)
                // buff[nbBlockFlvl-i-1]);
                unpack_and_encode_last_field_elem(a + nbBlockFlvl - i - 1,
                                                  (baseint_t *)in, inlen);

                field_mul_no_carry(acc_d, &acc, k + nbBlockFlvl - i - 1);
                carry_round(&acc, acc_d);
                for (j = 0; j < nbBlockFlvl - i - 1; ++j) {
                    field_mul_no_carry(acc_d + j + 1, a + j,
                                       k + nbBlockFlvl - i - 2 - j);
                    carry_round(a + j, acc_d + j + 1);
                    field_add(&acc, &acc, a + j);
                }
                field_add(&acc, &acc, a + nbBlockFlvl - i - 1);
                _carry_round(&acc, &acc);
                break;
            }
        }
        reduce(&acc, &acc);
        pack_field_elem((baseint_t *)tag_packed,
                        &acc); // transform limb representation of field element
                               // (unpacked) into packed field element
    }
    transform_field_elem(
        out, OUTPUTSIZE, tag_packed,
        BUFFSIZE); // transform from field element to byte missing?
}
