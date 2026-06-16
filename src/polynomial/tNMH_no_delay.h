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

#ifndef __MMH_NB_DELAY_H
#define __MMH_NB_DELAY_H
#include "../length_encoding.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

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

unsigned long long get_keylength(unsigned long long inlen) {
    const unsigned long long n =
        (((inlen / BLOCKSIZE) + !!(inlen % BLOCKSIZE) - 1) * BLOCKSIZE) /
        (SUPERBLOCKSIZE);
    if (n == 0) {
        return get_keylength_LE(inlen, (unsigned long long)SUPERKEYSIZE);
    }
    return get_keylength_LE(
        inlen,
        (unsigned long long)((2 * floor_log3(n) + 2) * KEYSIZE + SUPERKEYSIZE));
}

void tNMH_no_delay(unsigned char *out, const unsigned char *in,
                   unsigned long long inlen, const unsigned char *key,
                   unsigned long long keylen);

#endif
