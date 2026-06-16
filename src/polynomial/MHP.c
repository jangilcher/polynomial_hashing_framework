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
#include "boost/preprocessor/repetition/repeat.hpp"
#include "boost/preprocessor/repetition/repeat_from_to.hpp"
#include <stddef.h>
#include <string.h>

#if defined(INNERPOLY_H) && defined(INNERPOLY)
#include INNERPOLY_H
#endif

#if EXPLICIT_LENGTH_ENCODE
#include "../length_encoding.h"
#endif

#define WHILE_HANDLE_VAR(z, idx, data)                                         \
    INNERPOLY(&acc[NUM_KEYS - idx - 1], in + i, len, &inner_state,             \
              ((i + SUPERBLOCKSIZE) >= inlen));                                \
    i += SUPERBLOCKSIZE;                                                       \
    deg[NUM_KEYS - idx - 1] = 0;                                               \
    while ((i < inlen) && (deg[NUM_KEYS - idx - 1] < deg[NUM_KEYS - idx])) {   \
        field_mul_no_carry(&acc_d[NUM_KEYS - idx - 1],                         \
                           &acc[NUM_KEYS - idx - 1], &k[NUM_KEYS - idx - 1]);  \
        deg[NUM_KEYS - idx - 1]++;                                             \
        len = (i + SUPERBLOCKSIZE) > inlen ? inlen - i : SUPERBLOCKSIZE;

#define COMBINE_VAR(z, idx, data)                                              \
    field_add_mix(&acc_d[idx], &acc_d[idx], &acc[idx - 1]);                    \
    carry_round(&acc[idx], &acc_d[idx]);                                       \
    }

void MHP(unsigned char *out, const unsigned char *in, unsigned long long inlen,
         const unsigned char *key, unsigned long long keylen) {
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

    field_elem_t acc[NUM_KEYS] = {0};
    int deg[NUM_KEYS] = {0};
    dfield_elem_t acc_d[NUM_KEYS] = {0};
    field_elem_t k[NUM_KEYS] = {0};
    unsigned long long i = 0;

    INNER_STATE_T inner_state = INNER_STATE_ZERO;
    INNER_STATE_INIT(&inner_state, key);
    const unsigned char *outer_key = key + SUPERKEYSIZE;

#ifdef __GNUC__
#ifdef __clang__
#pragma unroll 65534
#else
#pragma GCC unroll 65534
#endif
#endif
    for (int j = 0; j < NUM_KEYS; j++) {
        unpack_and_encode_key(&k[j], (baseint_t *)(&outer_key[j * KEYSIZE]));
    }
    unsigned long long len =
        (i + SUPERBLOCKSIZE) > inlen ? inlen - i : SUPERBLOCKSIZE;
    INNERPOLY(&acc[NUM_KEYS - 1], in + i, len, &inner_state,
              ((i + SUPERBLOCKSIZE) >= inlen));
    i += SUPERBLOCKSIZE;
    deg[NUM_KEYS - 1] = 0;
    while (i < inlen) {
        field_mul_no_carry(&acc_d[NUM_KEYS - 1], &acc[NUM_KEYS - 1],
                           &k[NUM_KEYS - 1]);
        deg[NUM_KEYS - 1]++;
        len = (i + SUPERBLOCKSIZE) > inlen ? inlen - i : SUPERBLOCKSIZE;

        BOOST_PP_REPEAT_FROM_TO(1, NUM_KEYS, WHILE_HANDLE_VAR, 0);

        INNERPOLY(&acc[0], in + i, len, &inner_state,
                  ((i + SUPERBLOCKSIZE) >= inlen));
        i += SUPERBLOCKSIZE;
        field_add_mix(&acc_d[0], &acc_d[0], &acc[0]);
        carry_round(&acc[0], &acc_d[0]);
    }

    BOOST_PP_REPEAT_FROM_TO(1, NUM_KEYS, COMBINE_VAR, 0);

#if EXPLICIT_LENGTH_ENCODE
    LENGTH_ENCODING(&acc[NUM_KEYS - 1], &acc[NUM_KEYS - 1], key, keylen,
                    msglen);
#endif
    reduce(&acc[NUM_KEYS - 1], &acc[NUM_KEYS - 1]);
    pack_field_elem((baseint_t *)tag_packed, &acc[NUM_KEYS - 1]);
    transform_field_elem(out, OUTPUTSIZE, tag_packed, BUFFSIZE);
}
