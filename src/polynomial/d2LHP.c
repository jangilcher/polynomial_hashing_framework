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

#ifdef NUM_KEYS
#if NUM_KEYS == 1
#else
#warning "Currently Only supporting one key, i.e. univariate polynomial"
#undef NUM_KEYS
#define NUM_KEYS 1
#endif
#endif

#if EXPLICIT_LENGTH_ENCODE
#include "../length_encoding.h"
#endif

void d2LHP(unsigned char *out, const unsigned char *in,
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
    dfield_elem_t acc_d = {0};
    field_elem_t k = {0};
    field_elem_t k_pow = {0};
    unsigned long long i = 0;

    INNER_STATE_T inner_state = INNER_STATE_ZERO;
    INNER_STATE_INIT(&inner_state, key);
    const unsigned char *outer_key = key + SUPERKEYSIZE;

    // #ifdef __GNUC__
    // #ifdef __clang__
    // #pragma unroll 65534
    // #else
    // #pragma GCC unroll 65534
    // #endif
    // #endif
    unpack_and_encode_key(&k, (baseint_t *)(outer_key));
    // TODO: Some computing of keypowers
    memcpy(&k_pow, &k, sizeof(k_pow));
    uint64_t delta = (1ULL << (NB_KEYS)) - 1ULL;
    // printf("%"PRIu64"\n", delta);
    for (uint64_t i = 0; i < delta; i++) {
        field_mul(&k_pow, &k_pow, &k);
    }
    if (inlen >= SUPERBLOCKSIZE) {
        INNERPOLY(&acc, in + i, SUPERBLOCKSIZE, &inner_state,
                  ((i + SUPERBLOCKSIZE) == inlen));
        i += SUPERBLOCKSIZE;

        while (i + SUPERBLOCKSIZE <= inlen) {
            field_mul_no_carry(&acc_d, &acc, &k_pow);

            INNERPOLY(&acc, in + i, SUPERBLOCKSIZE, &inner_state,
                      ((i + SUPERBLOCKSIZE) == inlen));
            i += SUPERBLOCKSIZE;
            field_add_mix(&acc_d, &acc_d, &acc);
            carry_round(&acc, &acc_d);
        }
    }

    if (i < inlen) {
        if (inlen - i > SUPERBLOCKSIZE - BLOCKSIZE) {
            field_mul_no_carry(&acc_d, &acc, &k_pow);
            INNERPOLY(&acc, in + i, inlen - i, &inner_state, 1);
            i += SUPERBLOCKSIZE;
            field_add_mix(&acc_d, &acc_d, &acc);
            carry_round(&acc, &acc_d);
        } else {
            while (i + BLOCKSIZE < inlen) {
                field_mul_no_carry(&acc_d, &acc, &k);
                unpack_and_encode_field_elem(&acc, (baseint_t *)(in + i));
                field_add_mix(&acc_d, &acc_d, &acc);
                carry_round(&acc, &acc_d);
                i += BLOCKSIZE;
            }
            field_mul_no_carry(&acc_d, &acc, &k);
            unpack_and_encode_last_field_elem(&acc, (baseint_t *)(in + i),
                                              inlen - i);
            field_add_mix(&acc_d, &acc_d, &acc);
            carry_round(&acc, &acc_d);
        }
    }

#if EXPLICIT_LENGTH_ENCODE
    LENGTH_ENCODING(&acc, &acc, key, keylen, msglen);
#endif
    reduce(&acc, &acc);
    pack_field_elem((baseint_t *)tag_packed, &acc);
    transform_field_elem(out, OUTPUTSIZE, tag_packed, BUFFSIZE);
}
