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

#ifndef __LENGTH_ENCODING_H_
#define __LENGTH_ENCODING_H_
#include "field_arithmetic/field_arithmetic.h"
#include <stdio.h>

#ifndef LE_MIN_KEY
#define LE_MIN_KEY 0ULL
#endif
#ifndef LE_EXTRA_KEY
#define LE_EXTRA_KEY 0ULL
#endif

#define additive_length_encoding_MIN_KEY 2ULL
#define additive_length_encoding_EXTRA_KEY 2ULL
static inline __attribute__((always_inline)) void additive_length_encoding(
    field_elem_t *out, const field_elem_t *h, const unsigned char *key,
    const unsigned long long key_len, const unsigned long long msg_len) {
    dfield_elem_t tmp[2] = {0};
    field_elem_t keys[2] = {0};
    field_elem_t len = {0};
    unsigned char len_bytes[BUFFSIZE] = {0};
    memcpy(len_bytes, &msg_len, sizeof(unsigned long long));

    unpack_and_encode_key(&keys[0], (baseint_t *)(key + key_len - 2 * KEYSIZE));
    unpack_and_encode_key(&keys[1], (baseint_t *)(key + key_len - KEYSIZE));
    unpack_and_encode_field_elem(&len, (baseint_t *)len_bytes);
    field_mul_no_carry(&tmp[0], h, &keys[0]);
    field_mul_no_carry(&tmp[1], &len, &keys[1]);
    field_add_dbl(&tmp[0], &tmp[0], &tmp[1]);
    carry_round(out, &tmp[0]);
}

#define multiplicative_length_encoding_MIN_KEY 2ULL
#define multiplicative_length_encoding_EXTRA_KEY 2ULL
static inline __attribute__((always_inline)) void
multiplicative_length_encoding(field_elem_t *out, const field_elem_t *h,
                               const unsigned char *key,
                               const unsigned long long key_len,
                               const unsigned long long msg_len) {
    field_elem_t tmp[2] = {0};
    field_elem_t keys[2] = {0};
    field_elem_t len = {0};
    unsigned char len_bytes[BUFFSIZE] = {0};
    memcpy(len_bytes, &msg_len, sizeof(unsigned long long));

    unpack_and_encode_key(&keys[0], (baseint_t *)(key + key_len - 2 * KEYSIZE));
    unpack_and_encode_key(&keys[1], (baseint_t *)(key + key_len - KEYSIZE));
    unpack_and_encode_field_elem(&len, (baseint_t *)len_bytes);
    field_add(&tmp[0], h, &keys[0]);
    field_add(&tmp[1], &len, &keys[1]);
    field_mul(out, &tmp[0], &tmp[1]);
}

#define key_reuse_length_encoding_MIN_KEY 2ULL
#define key_reuse_length_encoding_EXTRA_KEY 1ULL
static inline __attribute__((always_inline)) void key_reuse_length_encoding(
    field_elem_t *out, const field_elem_t *h, const unsigned char *key,
    const unsigned long long key_len, const unsigned long long msg_len) {
    field_elem_t tmp[2] = {0};
    field_elem_t keys[2] = {0};
    field_elem_t len = {0};
    unsigned char len_bytes[BUFFSIZE] = {0};
    memcpy(len_bytes, &msg_len, sizeof(unsigned long long));
    unpack_and_encode_key(&keys[0], (baseint_t *)(key + key_len - KEYSIZE));
    unpack_and_encode_key(&keys[1], (baseint_t *)key);
    unpack_and_encode_field_elem(&len, (baseint_t *)len_bytes);
    field_add(&tmp[0], h, &keys[0]);
    field_add(&tmp[1], &len, &keys[1]);
    field_mul(out, &tmp[0], &tmp[1]);
}

#define simple_key_reuse_length_encoding_MIN_KEY 1ULL
#define simple_key_reuse_length_encoding_EXTRA_KEY 0ULL
static inline __attribute__((always_inline)) void
simple_key_reuse_length_encoding(field_elem_t *out, const field_elem_t *h,
                                 const unsigned char *key,
                                 const unsigned long long key_len,
                                 const unsigned long long msg_len) {
    field_elem_t tmp = {0};
    field_elem_t k = {0};
    field_elem_t len = {0};
    unsigned char len_bytes[BUFFSIZE] = {0};
    memcpy(len_bytes, &msg_len, sizeof(unsigned long long));
    unpack_and_encode_key(&k, (baseint_t *)key);
    unpack_and_encode_field_elem(&len, (baseint_t *)len_bytes);
    field_mul(&tmp, h, &k);
    field_add(&tmp, &len, &tmp);
    field_mul(out, &tmp, &k);
}

static inline __attribute__((always_inline)) unsigned long long
get_keylength_LE(const unsigned long long inlen,
                 const unsigned long long keylen) {
    if (keylen < LE_MIN_KEY * KEYSIZE) {
        return LE_MIN_KEY * KEYSIZE;
    }
    return keylen + LE_EXTRA_KEY * KEYSIZE;
}

#endif
