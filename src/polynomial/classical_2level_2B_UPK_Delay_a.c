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

// Classical 2 level with reduction after last addition
void classical_2level_2B_UPK_Delay_a(unsigned char *out,
                                     const unsigned char *in,
                                     unsigned long long inlen,
                                     const unsigned char *key) {
    if (inlen == 0) {
        memset(out, 0, OUTPUTSIZE);
        return;
    }
    field_elem_t acc = {0};
    dfield_elem_t acc_d[2] = {0};
    field_elem_t a[2];
    DECLARE_PC_ELEM_ARRAY(k, 2);
    unsigned char transkey[BUFFSIZE] = {0};
    unsigned char tag_packed[BUFFSIZE] = {0};
    unsigned long long double_blocksize = BLOCKSIZE + BLOCKSIZE;

    // Transform key from a byte array to one field elements
    transform_key(transkey, BUFFSIZE, key, KEYSIZE);
    UNPACK_PC_FIELD_ELEM_ARRAY(k, transkey, 0);
    INIT_PC_KEY(k, NOT_PRECOMPUTED(k));

    // process msg of only 1 block
    if (inlen <= BLOCKSIZE) {
        // transform msg block from bytes to field elements (packed)
        unpack_and_encode_last_field_elem(&acc, (baseint_t *)in, inlen);
        pack_field_elem((baseint_t *)tag_packed, &acc);
    } else if (inlen <= double_blocksize) {
        // transform field element(packed) into limb representation
        unpack_and_encode_field_elem(a, (baseint_t *)in);
        in += BLOCKSIZE;
        inlen -= BLOCKSIZE;
        unpack_and_encode_last_field_elem(a + 1, (baseint_t *)in, inlen);

        FIELD_MUL_PC_NO_CARRY(acc_d, a, k);
        field_add_mix(acc_d, acc_d, a + 1);
        carry_round(&acc, acc_d);
        reduce(&acc, &acc);
        pack_field_elem((baseint_t *)tag_packed, &acc);
    } else {
        field_mul(NOT_PRECOMPUTED(k) + 1, NOT_PRECOMPUTED(k),
                  NOT_PRECOMPUTED(k));
        INIT_PC_KEY(k + 1, NOT_PRECOMPUTED(k) + 1);

        // initialize accumulator
        // transform_msg(buff[0], BUFFSIZE, in, BLOCKSIZE);
        // unpack_field_elem(a, (baseint_t *) buff[0]);
        unpack_and_encode_field_elem(a, (baseint_t *)in);
        in += BLOCKSIZE;
        // transform_msg(buff[1], BUFFSIZE, in, BLOCKSIZE);
        // unpack_field_elem(a+1, (baseint_t *) buff[1]);
        unpack_and_encode_field_elem(a + 1, (baseint_t *)in);
        in += BLOCKSIZE;
        inlen -= double_blocksize;
        // field_mul_precomputed_no_carry(acc_d, a, k_p);
        FIELD_MUL_PC_NO_CARRY(acc_d, a, k);
        field_add_mix(acc_d, acc_d, a + 1);
        carry_round(&acc, acc_d);

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

            // field_mul_precomputed_no_carry(acc_d, &acc, k_p + 1);
            // field_mul_precomputed_no_carry(acc_d + 1, a, k_p);

            FIELD_MUL_PC_NO_CARRY(acc_d, &acc, k + 1);
            FIELD_MUL_PC_NO_CARRY(acc_d + 1, a, k);
            field_add_dbl(acc_d, acc_d, acc_d + 1);
            field_add_mix(acc_d, acc_d, a + 1);
            carry_round(&acc, acc_d);
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

            // field_mul_precomputed_no_carry(acc_d, &acc, k_p + 1);
            // field_mul_precomputed_no_carry(acc_d + 1, a, k_p);

            FIELD_MUL_PC_NO_CARRY(acc_d, &acc, k + 1);
            FIELD_MUL_PC_NO_CARRY(acc_d + 1, a, k);
            field_add_dbl(acc_d, acc_d, acc_d + 1);
            field_add_mix(acc_d, acc_d, a + 1);
            carry_round(&acc, acc_d);
        } else { // processing last msg block
            // transform_msg(buff[0], BUFFSIZE, in, inlen); // transform msg
            // block from bytes to field elements (packed) unpack_field_elem(a,
            // (baseint_t *) buff[0]); // transform field element(packed) into
            // limb representation
            unpack_and_encode_last_field_elem(a, (baseint_t *)in, inlen);
            // field_mul_precomputed_no_carry(acc_d, &acc, k_p);

            FIELD_MUL_PC_NO_CARRY(acc_d, &acc, k);
            field_add_mix(acc_d, acc_d, a);
            carry_round(&acc, acc_d);
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
