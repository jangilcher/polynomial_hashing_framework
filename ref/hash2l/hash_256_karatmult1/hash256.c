/*complete Hash2L for GF(2^256)*/
/*
###############################################################################
# Hash2L developers and authors:                                              #
#                                                                             #
# Debrup Chakraborty,  Indian Statistical Institute                           #
# Sebati Ghosh,        Indian Statistical Institute                           #
# Palash Sarkar,       Indian Statistical Institute                	      #
###############################################################################
#                                                                             #
###############################################################################
#                                                                             #
# Copyright (c) 2016, Debrup Chakraborty, Sebati Ghosh, Palash Sarkar         #
#                                                                             #
#                                                                             #
# Permission to use this code for Hash2L is granted.                          #
#                                                                             #
# Redistribution and use in source and binary forms, with or without          #
# modification, are permitted provided that the following conditions are      #
# met:                                                                        #
#                                                                             #
# * Redistributions of source code must retain the above copyright notice,    #
#   this list of conditions and the following disclaimer.                     #
#                                                                             #
# * Redistributions in binary form must reproduce the above copyright         #
#   notice, this list of conditions and the following disclaimer in the       #
#   documentation and/or other materials provided with the distribution.      #
#                                                                             #
# * The names of the contributors may not be used to endorse or promote       #
# products derived from this software without specific prior written          #
# permission.                                                                 #
#                                                                             #
###############################################################################
#                                                                             #
###############################################################################
# THIS SOFTWARE IS PROVIDED BY THE AUTHORS ""AS IS"" AND ANY                  #
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE           #
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR          #
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE        #
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 		      #
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 	      #
# OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR          	      #
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF      #
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING        #
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS          #
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                #
###############################################################################
*/
#include "timedefs.h"
#include <emmintrin.h>
#include <immintrin.h>
#include <smmintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wmmintrin.h>
#include <x86intrin.h>

#define HASHNUMBER 1

#include "hash.h"

#define MAX 10000 /*max number of 256 bit elements*/

#define STAMP                                                                  \
    ({                                                                         \
        unsigned res;                                                          \
        __asm__ __volatile__("rdtsc" : "=a"(res) : : "edx");                   \
        res;                                                                   \
    }) /* Time stamp */

int main(int argc, char *argv[]) {
    unsigned char k1[32], f1[3 * MAX];
    int i, j, k, l, length, finalblock, noOfBytes, fullBlocks, remaining,
        partial256, full256, pad, hornerLength = 0;

    __m128i aa1[MAX][2], key[2], digest[MAX][2], result[2];

    double tmpd;

    if (argc == 1) {
        printf("usage: %s msgLength\n", argv[0]);
        exit(0);
    } else if (argc == 2) {
        noOfBytes = atoi(argv[1]);
    }

    pad = 0;

    fullBlocks = noOfBytes / 992;

    remaining = noOfBytes % 992;

    if (remaining)
        finalblock = fullBlocks + 1;
    else
        finalblock = fullBlocks;

    partial256 = remaining % 32;
    if (partial256)
        pad = 32 - partial256;
    if (noOfBytes == 0) {
        pad = 32;
        finalblock = 1;
    }

    if (pad == 0)
        full256 = remaining / 32;
    else
        full256 = (remaining / 32) + 1;

    hornerLength = finalblock;

    for (i = 31; i >= 0; i--) {
        k1[i] = rand();
    }

    for (i = 0; i < noOfBytes; i++) {
        f1[i] = rand();
        i++;
    }
    for (i = noOfBytes; i < noOfBytes + pad; i++) {
        f1[i] = 0;
        i++;
    }

    key[0] = _mm_set_epi8(k1[15], k1[14], k1[13], k1[12], k1[11], k1[10], k1[9],
                          k1[8], k1[7], k1[6], k1[5], k1[4], k1[3], k1[2],
                          k1[1], k1[0]);
    key[1] = _mm_set_epi8(k1[31], k1[30], k1[29], k1[28], k1[27], k1[26],
                          k1[25], k1[24], k1[23], k1[22], k1[21], k1[20],
                          k1[19], k1[18], k1[17], k1[16]);

    printf("\ntotal no of bytes after padding %d\n", i);

    k = 0;
    for (j = 0; j < i / 16; j = j + 2) {
        aa1[k][0] = ((__m128i *)f1)[j];
        aa1[k][1] = ((__m128i *)f1)[j + 1];
        k++;
    }

    DO(HASH2L(aa1, key, digest, fullBlocks, remaining, noOfBytes, hornerLength,
              full256, result);
       ASSIGN(key, result));
    tmpd = ((median_get()) / (double)(N * noOfBytes));
    printf("\n median cycles in entire hash%d= %lf \n", HASHNUMBER, tmpd);

    printf("\nresult%d:\n", HASHNUMBER);
    printf("\n%x", _mm_extract_epi8(result[0], 0));
    printf("%x", _mm_extract_epi8(result[0], 1));
    printf("%x", _mm_extract_epi8(result[0], 2));
    printf("%x", _mm_extract_epi8(result[0], 3));
    printf("%x", _mm_extract_epi8(result[0], 4));
    printf("%x", _mm_extract_epi8(result[0], 5));
    printf("%x", _mm_extract_epi8(result[0], 6));
    printf("%x", _mm_extract_epi8(result[0], 7));
    printf("%x", _mm_extract_epi8(result[0], 8));
    printf("%x", _mm_extract_epi8(result[0], 9));
    printf("%x", _mm_extract_epi8(result[0], 10));
    printf("%x", _mm_extract_epi8(result[0], 11));
    printf("%x", _mm_extract_epi8(result[0], 12));
    printf("%x", _mm_extract_epi8(result[0], 13));
    printf("%x", _mm_extract_epi8(result[0], 14));
    printf("%x", _mm_extract_epi8(result[0], 15));
    printf("\n%x", _mm_extract_epi8(result[1], 0));
    printf("%x", _mm_extract_epi8(result[1], 1));
    printf("%x", _mm_extract_epi8(result[1], 2));
    printf("%x", _mm_extract_epi8(result[1], 3));
    printf("%x", _mm_extract_epi8(result[1], 4));
    printf("%x", _mm_extract_epi8(result[1], 5));
    printf("%x", _mm_extract_epi8(result[1], 6));
    printf("%x", _mm_extract_epi8(result[1], 7));
    printf("%x", _mm_extract_epi8(result[1], 8));
    printf("%x", _mm_extract_epi8(result[1], 9));
    printf("%x", _mm_extract_epi8(result[1], 10));
    printf("%x", _mm_extract_epi8(result[1], 11));
    printf("%x", _mm_extract_epi8(result[1], 12));
    printf("%x", _mm_extract_epi8(result[1], 13));
    printf("%x", _mm_extract_epi8(result[1], 14));
    printf("%x", _mm_extract_epi8(result[1], 15));
    printf("\n");

    return 0;
}
