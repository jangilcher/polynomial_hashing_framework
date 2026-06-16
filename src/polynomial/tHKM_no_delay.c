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
#include "boost/preprocessor/arithmetic/add.hpp"
#include "boost/preprocessor/arithmetic/mul.hpp"
#include "boost/preprocessor/arithmetic/sub.hpp"
#include "boost/preprocessor/repetition/repeat.hpp"
#include "boost/preprocessor/repetition/repeat_from_to.hpp"
#include <stddef.h>
#include <string.h>
#if EXPLICIT_LENGTH_ENCODE
#include "../length_encoding.h"
#endif
#if defined(INNERPOLY_H) && defined(INNERPOLY)
#include INNERPOLY_H
#endif

#define DEBUGPRINTF(...) // printf(__VA_ARGS__)
#define DEBUGPRINT_FIELD_ELEM(el)                                              \
    DEBUGPRINTF(#el ".val[0]: %" PRIx64 "\n", el.val[0]);                      \
    DEBUGPRINTF(#el ".val[1]: %" PRIx64 "\n", el.val[1]);

// If x > 0: Returns exponent of highest power of 3 that divides x
// UNDEFINED if x == 0
static inline int highest3Power(unsigned long long x, int *power) {
    int i = 0;
    int y = 1;
    while (!(x % (y * 3))) {
        i += 1;
        y *= 3;
    }
    if (power) {
        *power = y;
    }
    return i;
}

// If x > 0: Returns exponent of highest power of 2 smaller than x
// UNDEFINED if x == 0
static inline int floor_log2(uint64_t x) { return 63 - __builtin_clzll(x); }

// If x > 0: Returns exponent of highest power of 3 smaller than x
// UNDEFINED if x == 0
static inline int floor_log3(uint64_t x) {
    const int guess[64] = {0,  0,  1,  1,  2,  3,  3,  4,  5,  5,  6,  6,  7,
                           8,  8,  9,  10, 10, 11, 11, 12, 13, 13, 14, 15, 15,
                           16, 17, 17, 18, 18, 19, 20, 20, 21, 22, 22, 23, 23,
                           24, 25, 25, 26, 27, 27, 28, 29, 29, 30, 30, 31, 32,
                           32, 33, 34, 34, 35, 35, 36, 37, 37, 38, 39, 39};
    const uint64_t powers3[41] = {1,
                                  3,
                                  9,
                                  27,
                                  81,
                                  243,
                                  729,
                                  2187,
                                  6561,
                                  19683,
                                  59049,
                                  177147,
                                  531441,
                                  1594323,
                                  4782969,
                                  14348907,
                                  43046721,
                                  129140163,
                                  387420489,
                                  1162261467,
                                  3486784401,
                                  10460353203,
                                  31381059609,
                                  94143178827,
                                  282429536481,
                                  847288609443,
                                  2541865828329,
                                  7625597484987,
                                  22876792454961,
                                  68630377364883,
                                  205891132094649,
                                  617673396283947,
                                  1853020188851841,
                                  5559060566555523,
                                  16677181699666569,
                                  50031545098999707,
                                  150094635296999121,
                                  450283905890997363,
                                  1350851717672992089,
                                  4052555153018976267ULL,
                                  12157665459056928801ULL};

    int log = guess[floor_log2(x)];
    return log + (x >= powers3[log + 1]);
}

void tHKM_no_delay(unsigned char *out, const unsigned char *in,
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

    if (inlen <= SUPERBLOCKSIZE) {
        field_elem_t acc = {0};
        INNERPOLY(&acc, in, inlen, key, 1);
#if EXPLICIT_LENGTH_ENCODE
        LENGTH_ENCODING(&acc, &acc, key, keylen, msglen);
        reduce(&acc, &acc);
#endif
        pack_field_elem((baseint_t *)tag_packed, &acc);
    } else {
        uint64_t n = inlen / SUPERBLOCKSIZE;
        uint64_t s = floor_log3(n);
        int power = 0;
        field_elem_t tmp = {0};
        field_elem_t *acc = calloc(2 * s + 3, sizeof(field_elem_t));
        if (!acc)
            exit(-1);
        int stack_idx = 0;
        field_elem_t k[1] = {0};
        int i = 0;
        for (i = 0; i < n; i++) {
            INNERPOLY(&acc[stack_idx], in, SUPERBLOCKSIZE, key,
                      inlen == SUPERBLOCKSIZE);
            in += SUPERBLOCKSIZE;
            inlen -= SUPERBLOCKSIZE;
            stack_idx++;
            if (i % 3 == 2) {
                s = highest3Power(i + 1, &power);
                for (int j = 0; j < s; j++) {
                    unpack_and_encode_key(
                        &k[0], (baseint_t *)(key + SUPERKEYSIZE + j * KEYSIZE));
                    stack_idx -= 3;
                    field_add_reduce(&tmp, &acc[stack_idx + 1], &k[0]);
                    field_mul_reduce(&acc[stack_idx], &acc[stack_idx], &tmp);
                    field_add_reduce(&acc[stack_idx], &acc[stack_idx],
                                     &acc[stack_idx + 2]);
                    acc[stack_idx + 1] = (field_elem_t){0};
                    acc[stack_idx + 2] = (field_elem_t){0};
                    stack_idx++;
                }
            }
        }
        if (inlen > 0) {
            INNERPOLY(&acc[stack_idx], in, inlen, key, 1);
            stack_idx++;
            {
                if (i % 3 == 2) {
                    s = highest3Power(i + 1, &power);
                    for (int j = 0; j < s; j++) {
                        unpack_and_encode_key(
                            &k[0],
                            (baseint_t *)(key + SUPERKEYSIZE + j * KEYSIZE));
                        stack_idx -= 3;
                        field_add_reduce(&tmp, &acc[stack_idx + 1], &k[0]);
                        field_mul_reduce(&acc[stack_idx], &acc[stack_idx],
                                         &tmp);
                        field_add_reduce(&acc[stack_idx], &acc[stack_idx],
                                         &acc[stack_idx + 2]);
                        acc[stack_idx + 1] = (field_elem_t){0};
                        acc[stack_idx + 2] = (field_elem_t){0};
                        stack_idx++;
                    }
                }
            }
            i++;
        }

        int ii = i - 1;
        int level = 0;
        while (ii % 3 == 2) {
            ii /= 3;
            level++;
        }

        while (ii > 0) {
            unpack_and_encode_key(
                &k[0], (baseint_t *)(key + SUPERKEYSIZE + level * KEYSIZE));
            if (ii % 3 == 0) {
                stack_idx -= 1;
                stack_idx++;
            } else if (ii % 3 == 1) {
                stack_idx -= 2;
                field_add_reduce(&tmp, &acc[stack_idx + 1], &k[0]);
                field_mul_reduce(&acc[stack_idx], &acc[stack_idx], &tmp);
                acc[stack_idx + 1] = (field_elem_t){0};
                stack_idx++;
            } else if (ii % 3 == 2) {
                stack_idx -= 3;
                field_add_reduce(&tmp, &acc[stack_idx + 1], &k[0]);
                field_mul_reduce(&acc[stack_idx], &acc[stack_idx], &tmp);
                field_add_reduce(&acc[stack_idx], &acc[stack_idx],
                                 &acc[stack_idx + 2]);
                acc[stack_idx + 1] = (field_elem_t){0};
                acc[stack_idx + 2] = (field_elem_t){0};
                stack_idx++;
            }
            ii /= 3;
            level++;
        }
#if EXPLICIT_LENGTH_ENCODE
        LENGTH_ENCODING(&acc[stack_idx - 1], &acc[stack_idx - 1], key, keylen,
                        inlen);
        reduce(&acc[stack_idx - 1], &acc[stack_idx - 1]);
#endif
        pack_field_elem((baseint_t *)tag_packed, &acc[stack_idx - 1]);
        free(acc);
    }

    transform_field_elem(out, OUTPUTSIZE, tag_packed, BUFFSIZE);
}
