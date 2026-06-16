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

// PARAM0 indicates if reduction after every multiplication (NB_BLOCK_DELAY=0) or after every 2 multiplications (NB_BLOCK_DELAY=1)
#ifdef OUTER
#ifdef OUTER_PARAM0
#define NB_BLOCK_DELAY OUTER_PARAM0
#else
#define NB_BLOCK_DELAY 0
#endif
#else
#ifdef INNER_PARAM0
#define NB_BLOCK_DELAY INNER_PARAM0
#else
#define NB_BLOCK_DELAY 0
#endif
#endif

#if NB_BLOCK_DELAY
#define INIT_ACC \
    dfield_elem_t T[lg];\
    dfield_elem_t a_d;
#define MULT_THEN_ADD   \
    field_mul_no_carry(&a_d, a, a + 1); \
    field_add_mix(&a_d, &a_d, a + 2);
#define ADD_TO_ACC  \
    field_add_dbl(&a_d, &a_d, T + i);
#define CARRY \
    carry_round(a, &a_d);
#define MULT_TO_TREE    \
    field_mul_no_carry(T + sp - 2, a, a + 3);
#define REINIT_ACC \
    a_d = (dfield_elem_t){0};
#define REINIT_ACC_B \
    a_d = (dfield_elem_t){0};\
    field_add_mix(&a_d, &a_d, a);
#define LAST_MUL_ADD \
    field_mul_no_carry(&a_d, a, k); \
    field_add_mix(&a_d, &a_d, a + 1);
#define LAST_CARRY carry_round(out, &a_d);
#else
#define INIT_ACC \
    field_elem_t T[lg];
#define MULT_THEN_ADD   \
    field_mul(a, a, a + 1); \
    field_add(a, a, a + 2);
#define ADD_TO_ACC  \
    field_add(a, a, T + i);
#define CARRY
#define MULT_TO_TREE    \
    field_mul(T + sp - 2, a, a + 3);
#define REINIT_ACC \
    a[0] = (field_elem_t){0};
#define REINIT_ACC_B
#define LAST_MUL_ADD \
    field_mul(a, a, k); \
    field_add(a, a, a + 1);
#define LAST_CARRY _carry_round(out, a);
#endif

#define LOG2(X)                                                                \
    ((unsigned)(8 * sizeof(unsigned long long) - __builtin_clzll((X)) - 1))


#if defined(OUTER) || defined(NO_INNER_CACHE)
#define UNPACK_AND_ENCODE_KEY   \
    unsigned int lg = LOG2(noOfBlocks); \
    field_elem_t *k = calloc(lg + 1, sizeof(field_elem_t));\
    if (!k) exit(-1); \
    unpack_and_encode_key(k, (baseint_t *) key);\
    if (noOfBlocks > 2) \
        for (i = 0; i < lg; ++i) {field_sqr(k + i + 1, k + i);}
#define FREE_KEY_ALLOC free(k);
#else
#define NB_KEYS LOG2(NB_SUPERBLOCKS)+1
#define UNPACK_AND_ENCODE_KEY \
    unsigned int lg = NB_KEYS-1; \
    field_elem_t *k = (field_elem_t *)state->key;
#define FREE_KEY_ALLOC

typedef struct BRW_NB_Delay_state {
    field_elem_t key[NB_KEYS];
} BRW_NB_Delay_inner_state_t;

INLINE void BRW_NB_Delay_inner_init_state(BRW_NB_Delay_inner_state_t *state,
                                          const unsigned char *key) {
    field_elem_t *st_key = (field_elem_t *)state->key;
    unpack_and_encode_key(st_key, (baseint_t *) key);
    if (NB_SUPERBLOCKS > 2)
        for (int i = 0; i < NB_KEYS-1; ++i) {field_sqr(st_key + i + 1, st_key + i);}
}

#define INNER_STATE_ZERO {0}
#define INNER_STATE_T BRW_NB_Delay_inner_state_t
#define INNER_STATE_INIT BRW_NB_Delay_inner_init_state

#endif


#if defined(OUTER) || defined(NO_INNER_CACHE)
INLINE void BRW_NB_Delay_inner(field_elem_t *out, const unsigned char *in,
                               unsigned long long inlen,
                               const unsigned char *key, int last)
#else
INLINE void BRW_NB_Delay_inner(field_elem_t *out, const unsigned char *in,
                               unsigned long long inlen,
                               BRW_NB_Delay_inner_state_t *state, int last)
#endif
{
    if (inlen == 0) {
        memset(out, 0, OUTPUTSIZE);
        return;
    }
    unsigned long long blkctr = 0;
    field_elem_t a[4]; // temporary field element representing the block being processed
    unsigned long long noOfBlocks = inlen / BLOCKSIZE + (inlen % BLOCKSIZE != 0);
    unsigned int i, sp;

    UNPACK_AND_ENCODE_KEY;
    
    INIT_ACC;

    while (inlen > blkctr + 4 * BLOCKSIZE) {
        for (i = 0; i < 4; ++i) {
            unpack_and_encode_field_elem(a + i, (baseint_t *)(in + blkctr));
            blkctr += BLOCKSIZE;
        }
        field_add(a, a, k);
        field_add(a + 1, a + 1, k + 1);
        MULT_THEN_ADD;

        sp = __builtin_ctz(blkctr) - __builtin_ctz(BLOCKSIZE);
        for (i = 0; i + 2 < sp; ++i) {
            ADD_TO_ACC;
        }
        CARRY;
        field_add(a + 3, a + 3, k + sp);
        MULT_TO_TREE;
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
        MULT_THEN_ADD;

        sp = __builtin_ctz(blkctr) - __builtin_ctz(BLOCKSIZE); // s+2
        for (i = 0; i + 2 < sp; ++i) {
            ADD_TO_ACC;
        }
        CARRY;
        field_add(a + 3, a + 3, k + sp);
        MULT_TO_TREE;
        REINIT_ACC;
        break;
    case 1:
        unpack_and_encode_last_field_elem(a, (baseint_t *)(in + blkctr),
                                          ((inlen - 1) % BLOCKSIZE) + 1);
        REINIT_ACC_B;
        break;

    case 2:
        unpack_and_encode_field_elem(a, (baseint_t *)(in + blkctr));
        blkctr += BLOCKSIZE;
        unpack_and_encode_last_field_elem(a + 1, (baseint_t *)(in + blkctr),
                                          ((inlen - 1) % BLOCKSIZE) + 1);
        LAST_MUL_ADD;
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
        MULT_THEN_ADD;
        break;
    }

    noOfBlocks >>= 2;
    for (i = 0; noOfBlocks > 0 ; ++i, noOfBlocks >>= 1) {
        if ((noOfBlocks & 1) == 1) {
            ADD_TO_ACC;
        }
    }

    LAST_CARRY;
    FREE_KEY_ALLOC;
}
