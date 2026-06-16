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

#ifdef ALWAYS_INLINE_INNER
#define INLINE static inline __attribute__((always_inline))
#else
#define INLINE static inline
#endif

// PARAM0 defines when the carry occur
// PARAM1 defines whether carry befor multiplication
#ifdef OUTER
#if OUTER_PARAM0
#define DO_EARLY_CARRY
#endif
#if OUTER_PARAM1
#define CARRY_ADD
#endif
#if INNER_PARAM0
#define DO_EARLY_CARRY
#endif
#if INNER_PARAM1
#define CARRY_ADD
#endif
#endif

#ifdef DO_EARLY_CARRY
#define FIELD_MUL_AND_ADD                                                      \
    field_mul_no_carry(&acc_d, &acc, a);                                       \
    carry_round(&acc, &acc_d);                                                 \
    field_add(&acc, &acc, a + 1);
#define FINAL_CARRY _carry_round(&acc, &acc);
#else
#define FIELD_MUL_AND_ADD                                                      \
    field_mul_no_carry(&acc_d, &acc, a);                                       \
    field_add_mix(&acc_d, &acc_d, a + 1);                                      \
    carry_round(&acc, &acc_d);
#define FINAL_CARRY
#endif

#if defined(OUTER) || defined(NO_INNER_CACHE)
#define UNPACK_AND_ENCODE_KEY(res, k)                                          \
    unpack_and_encode_key((res), (k));                                         \
    key += KEYSIZE;

#define FINALIZE_BLOCK
#else
#define UNPACK_AND_ENCODE_KEY(res, k)

#define NB_KEYS ((NB_SUPERBLOCKS / 2) + 1)
typedef struct HKM_state {
    field_elem_t key[NB_KEYS];
} HKM_inner_state_t;
#define INNER_STATE_T HKM_inner_state_t
#define INNER_STATE_INIT HKM_inner_init_state

INLINE void HKM_inner_init_state(HKM_inner_state_t *state,
                                 const unsigned char *key) {
#if NB_KEYS == NB_SUPERKEYS
    for (int i = 0; i < NB_SUPERKEYS; i++) {
        unpack_and_encode_key(&state->key[i], (baseint_t *)key);
        key += KEYSIZE;
    }
#else
    field_elem_t *st_key = (field_elem_t *)state->key;
    unpack_and_encode_key((field_elem_t *)st_key, (baseint_t *)key);

#ifdef __GNUC__
#ifdef __clang__
#pragma unroll 65534
#else
#pragma GCC unroll 65534
#endif
#endif
    for (int j = 1; j < NB_KEYS; j++) {
        field_mul(&st_key[j], &st_key[j - 1], &st_key[0]);
    }
#endif
}
#if SUPERKEYSIZE
#define INNER_STATE_ZERO                                                       \
    { 0 }
#else
#define INNER_STATE_ZERO                                                       \
    {}
#endif
#define FINALIZE_BLOCK k++;
#endif

#if defined(OUTER) || defined(NO_INNER_CACHE)
INLINE void HKM_inner(field_elem_t *out, const unsigned char *in,
                      unsigned long long inlen, const unsigned char *key,
                      int last)
#else
INLINE void HKM_inner(field_elem_t *out, const unsigned char *in,
                      unsigned long long inlen, HKM_inner_state_t *state,
                      int last)
#endif

{
    if (inlen == 0) {
        memset(out, 0, sizeof(field_elem_t));
        return;
    }
    // field_elem_t acc = {0};
#define acc *out
    dfield_elem_t acc_d = {0};
    field_elem_t a[2];
#if defined(OUTER) || defined(NO_INNER_CACHE)
    field_elem_t keys;
    field_elem_t *k = &keys;
#else
    field_elem_t *k = (field_elem_t *)state->key;
#endif

    // initialize Polynomial/Accumulator
    if (inlen > BLOCKSIZE) {
        unpack_and_encode_field_elem(a, (baseint_t *)in);
        in += BLOCKSIZE;
        UNPACK_AND_ENCODE_KEY(k, (baseint_t *)key);
        inlen -= BLOCKSIZE;
        field_add(&acc, a, k);
#ifdef CARRY_ADD
        _carry_round(&acc, &acc);
#endif
        FINALIZE_BLOCK;
    } else {
        acc = field_elem_get_one();
    }

    // Process blocks by 2
    while (inlen > 2 * BLOCKSIZE) {
        unpack_and_encode_field_elem(a, (baseint_t *)in);
        UNPACK_AND_ENCODE_KEY(k, (baseint_t *)key);
        in += BLOCKSIZE;
        unpack_and_encode_field_elem(a + 1, (baseint_t *)in);
        in += BLOCKSIZE;
        inlen -= 2 * BLOCKSIZE;

        field_add(a, a, k);
#ifdef CARRY_ADD
        _carry_round(a, a);
#endif
        FIELD_MUL_AND_ADD;
        FINALIZE_BLOCK;
    }

    // Process last two blocks
    if (inlen > BLOCKSIZE) {
        unpack_and_encode_field_elem(a, (baseint_t *)in);
        UNPACK_AND_ENCODE_KEY(k, (baseint_t *)key);
        in += BLOCKSIZE;
        inlen -= BLOCKSIZE;
        UNPACK_AND_ENCODE_LAST_FIELD_ELEM(a + 1, (baseint_t *)in, inlen);

        field_add(a, a, k);
#ifdef CARRY_ADD
        _carry_round(a, a);
#endif
        FIELD_MUL_AND_ADD;
        FINAL_CARRY;
        FINALIZE_BLOCK;
    } else if (inlen > 0) { // Process possible last block
        UNPACK_AND_ENCODE_LAST_FIELD_ELEM(a, (baseint_t *)in, inlen);
        UNPACK_AND_ENCODE_KEY(k, (baseint_t *)key);

        field_add(a, a, k);
#ifdef CARRY_ADD
        _carry_round(a, a);
#endif
        field_mul_no_carry(&acc_d, &acc, a);
        carry_round(&acc, &acc_d);
        FINALIZE_BLOCK;
    }

    //_carry_round(&acc, &acc);
    // reduce(out, &acc);
#undef acc
}
