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

// reduction after last multiplication
void DCHMHorner_2level_15B_PK_Delay_b(unsigned char *out,
                                      const unsigned char *in,
                                      unsigned long long inlen,
                                      const unsigned char *key) {
    if (inlen == 0) {
        memset(out, 0, OUTPUTSIZE);
        return;
    }
    unsigned int nbBlockFlvl = 15;
    unsigned int i, j;
    field_elem_t acc = {0};
    dfield_elem_t acc_d = {0};
    field_elem_t a[nbBlockFlvl]; // temporary field element representing the
                                 // block being processed
    dfield_elem_t a_d[nbBlockFlvl];
    field_elem_t k[nbBlockFlvl];
    field_elem_precomputed_t k_p[nbBlockFlvl];
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
    precompute_factor(k_p, k);

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
                for (j = 0; j < i - 1; ++j) {
                    // field_mul(k+j+1, k+j ,k);
                    field_mul_precomputed(k + j + 1, k + j, k_p);
                    precompute_factor(k_p + j + 1, k + j + 1);
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

                for (j = 0; j < i - 1; j += 2) {
                    field_add(a + j, a + j, k + j);
                    field_add(a + j + 1, a + j + 1, k + j + 1);
                    field_mul_no_carry(a_d + j, a + j, a + j + 1);
                    field_add_dbl(&acc_d, &acc_d, a_d + j);
                }
                carry_round(&acc, &acc_d);
                if (j == i - 1) {
                    field_add(&acc, &acc, a + j);
                    _carry_round(&acc, &acc);
                }
                reduce(&acc, &acc);
                pack_field_elem((baseint_t *)tag_packed, &acc);
                break;
            }
        }
    } else { // process msg of more than nbBlockFlvl Blocks
        // if (inlen > nbBlockFlvl_blocksize) {
        // compute key powers for parallel horner
        for (i = 0; i < nbBlockFlvl - 1; ++i) {
            // field_mul(k+i+1, k+i ,k);
            field_mul_precomputed(k + i + 1, k + i, k_p);
            precompute_factor(k_p + i + 1, k + i + 1);
        }
        // initialize accumulator
        for (i = 0; i < nbBlockFlvl; ++i) {
            // transform_msg(buff[i], BUFFSIZE, in, BLOCKSIZE);
            // unpack_field_elem(a+i, (baseint_t *) buff[i]);
            unpack_and_encode_field_elem(a + i, (baseint_t *)in);
            in += BLOCKSIZE;
        }
        inlen -= nbBlockFlvl_blocksize;
        for (i = 0; i < nbBlockFlvl - 1; i += 2) {
            field_add(a + i, a + i, k + i);
            field_add(a + i + 1, a + i + 1, k + i + 1);
            field_mul_no_carry(a_d + i, a + i, a + i + 1);
            field_add_dbl(&acc_d, &acc_d, a_d + i);
        }
        carry_round(&acc, &acc_d);
        field_add(&acc, &acc, a + nbBlockFlvl - 1);

        while (inlen > nbBlockFlvl_blocksize) {
            for (i = 0; i < nbBlockFlvl; ++i) {
                // transform_msg(buff[i], BUFFSIZE, in, BLOCKSIZE);
                // unpack_field_elem(a+i, (baseint_t *) buff[i]);
                unpack_and_encode_field_elem(a + i, (baseint_t *)in);
                in += BLOCKSIZE;
            }
            inlen -= nbBlockFlvl_blocksize;

            // field_mul_no_carry(&acc_d, &acc, k+nbBlockFlvl-1);
            field_mul_precomputed_no_carry(&acc_d, &acc, k_p + nbBlockFlvl - 1);
            for (i = 0; i < nbBlockFlvl - 1; i += 2) {
                field_add(a + i, a + i, k + i);
                field_add(a + i + 1, a + i + 1, k + i + 1);
                field_mul_no_carry(a_d + i, a + i, a + i + 1);
                field_add_dbl(&acc_d, &acc_d, a_d + i);
            }
            carry_round(&acc, &acc_d);
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

                // field_mul_no_carry(&acc_d, &acc, k+nbBlockFlvl-1);// need to
                // check if correct power (or can decrease?)
                field_mul_precomputed_no_carry(
                    &acc_d, &acc,
                    k_p + nbBlockFlvl -
                        1); // need to check if correct power (or can decrease?)
                for (j = 0; j < nbBlockFlvl - i - 1; j += 2) {
                    field_add(a + j, a + j, k + j);
                    field_add(a + j + 1, a + j + 1, k + j + 1);
                    field_mul_no_carry(a_d + j, a + j, a + j + 1);
                    field_add_dbl(&acc_d, &acc_d, a_d + j);
                }
                carry_round(&acc, &acc_d);
                if (j == nbBlockFlvl - i - 1) {
                    field_add(&acc, &acc, a + j);
                    _carry_round(&acc, &acc);
                }
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
