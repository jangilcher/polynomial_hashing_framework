// MIT License
//
// Copyright (c) 2025 Jan Gilcher, Jérôme Govinden
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

#if EXPLICIT_LENGTH_ENCODE
#include "../length_encoding.h"
#endif

void tHorner(unsigned char *out, const unsigned char *in,
             unsigned long long inlen, const unsigned char *key,
             unsigned long long keylen) {
#if EXPLICIT_LENGTH_ENCODE
    const unsigned long long msglen = inlen;
#endif
    unsigned char tag_packed[BUFFSIZE] = {0};
    if (inlen == 0) {
#if EXPLICIT_LENGTH_ENCODE
        field_elem_t acc = {0};
        LENGTH_ENCODING(&acc, &acc, key, keylen, msglen);
        reduce(&acc, &acc);
        pack_field_elem((baseint_t *)tag_packed, &acc);
        transform_field_elem(out, OUTPUTSIZE, tag_packed, BUFFSIZE);
#else
        memset(out, 0, OUTPUTSIZE);
#endif
        return;
    }

    field_elem_t acc = {0};
    field_elem_t a = {0};
    dfield_elem_t acc_d = {0};
    DECLARE_PC_ELEM_ARRAY(k, 2) = {0};

    unpack_and_encode_key(k, (baseint_t *)key);
    INIT_PC_KEY(k, NOT_PRECOMPUTED(k));
    FIELD_MUL_PC_NO_CARRY(&acc_d, NOT_PRECOMPUTED(k), k);
    carry_round(NOT_PRECOMPUTED(k) + 1, &acc_d);
    INIT_PC_KEY(k + 1, NOT_PRECOMPUTED(k) + 1);
    for (int i = 2; i < (SUPERBLOCKSIZE / BLOCKSIZE); i++) {
        FIELD_MUL_PC_NO_CARRY(&acc_d, NOT_PRECOMPUTED(k) + 1, k);
        carry_round(NOT_PRECOMPUTED(k) + 1, &acc_d);
        INIT_PC_KEY(k + 1, NOT_PRECOMPUTED(k) + 1);
    }
    while (inlen > SUPERBLOCKSIZE) {
        INNERPOLY(&a, in, SUPERBLOCKSIZE, key, 0);
        FIELD_MUL_PC_NO_CARRY(&acc_d, &acc, k + 1);
        field_add_mix(&acc_d, &acc_d, &a);
        carry_round(&acc, &acc_d);
        inlen -= SUPERBLOCKSIZE;
        in += SUPERBLOCKSIZE;
    }
    INNERPOLY(&a, in, inlen, key, 1);
    FIELD_MUL_PC_NO_CARRY(&acc_d, &acc, k + 1);
    field_add_mix(&acc_d, &acc_d, &a);

    carry_round(&acc, &acc_d);
    inlen -= SUPERBLOCKSIZE;
    in += SUPERBLOCKSIZE;

#if EXPLICIT_LENGTH_ENCODE
    LENGTH_ENCODING(&acc, &acc, key, keylen, msglen);
#endif
    reduce(&acc, &acc);
    pack_field_elem((baseint_t *)tag_packed, &acc);
    transform_field_elem(out, OUTPUTSIZE, tag_packed, BUFFSIZE);
}
