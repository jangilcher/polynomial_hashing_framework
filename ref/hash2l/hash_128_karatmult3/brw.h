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
#ifndef BRW128_H_
#define BRW128_H_

#include "choicefield.h"

#define MULTKEYUPTO2 mult(key, key, key2);

#define MULTKEYUPTO4                                                           \
    mult(key, key, key2);                                                      \
    mult(key2, key2, key4);

#define MULTKEYUPTO8                                                           \
    mult(key, key, key2);                                                      \
    mult(key2, key2, key4);                                                    \
    mult(key4, key4, key8);

#define MULTKEYUPTO16                                                          \
    mult(key, key, key2);                                                      \
    mult(key2, key2, key4);                                                    \
    mult(key4, key4, key8);                                                    \
    mult(key8, key8, key16);

#define MULTKEYUPTO32                                                          \
    mult(key, key, key2);                                                      \
    mult(key2, key2, key4);                                                    \
    mult(key4, key4, key8);                                                    \
    mult(key8, key8, key16);                                                   \
    mult(key16, key16, key32);

#define evalbrwpar3(aa, key, dig)                                              \
    ({                                                                         \
        __m128i new1[2], new2[2], new26[2], new10[2];                          \
        /* new19[2], new23[2], new27[2], new31[2], new32[2], new34[2],         \
         * new33[2], new39[2], new35[2], new40[2], new42[2], sum32[2],         \
         * sum37[2], sum38[2], sum41[2], sum42[2];*/                           \
        XOR(aa[0], key, btmp1);                                                \
        XOR(aa[1], key2, btmp2);                                               \
        /*XOR(aa[4],key,btmp5);                                                \
        XOR(aa[5],key2,btmp6);                                                 \
        XOR(aa[7],key8,btmp8);*/                                               \
        XOR(aa[8], key, btmp9);                                                \
        XOR(aa[9], key2, btmp10);                                              \
        XOR(aa[16], key, btmp17);                                              \
        XOR(aa[17], key2, btmp18);                                             \
        karatsuba3(btmp2, btmp1, new1, xx, btmp10, btmp9, new2, xx1, btmp18,   \
                   btmp17, new26, xx2);                                        \
        XOR(aa[2], new1[0], new1[0]);                                          \
        XOR(aa[10], new2[0], new2[0]);                                         \
        XOR(aa[18], new26[0], new26[0]);                                       \
        reductionbymult3(new1, xx, btmp2, new2, xx1, btmp10, new26, xx2,       \
                         btmp18);                                              \
        XOR(aa[4], key, btmp1);                                                \
        XOR(aa[5], key2, btmp9);                                               \
        XOR(aa[3], key4, btmp17);                                              \
        XOR(aa[24], key, btmp25);                                              \
        XOR(aa[25], key2, btmp26);                                             \
        karatsuba3(btmp1, btmp9, new1, xx, btmp2, btmp17, new2, xx1, btmp26,   \
                   btmp25, new26, xx2);                                        \
        XOR(aa[26], new26[0], new26[0]);                                       \
        reductionbymult(new26, xx2, btmp26);                                   \
        XOR(aa[6], new1[0], new1[0]);                                          \
        /*XOR(new1[0],new2[0],new1[0]);                                        \
        XOR(new1[1],new2[1],new1[1]);*/                                        \
        XORNEW(new1, new2, new1);                                              \
        XOR(xx, xx1, xx);                                                      \
        reductionbymult(new1, xx, btmp1);                                      \
        XOR(aa[7], key8, btmp9);                                               \
        XOR(aa[11], key4, btmp2);                                              \
        XOR(aa[12], key, btmp17);                                              \
        XOR(aa[13], key2, btmp25);                                             \
        /*XOR(aa[11],key4,btmp12);*/                                           \
        karatsuba3(btmp1, btmp9, new1, xx, btmp2, btmp10, new2, xx1, btmp25,   \
                   btmp17, new26, xx2);                                        \
        XOR(aa[14], new26[0], new26[0]);                                       \
        /*XOR(new2[0],new26[0],new26[0]);                                      \
        XOR(new2[1],new26[1],new26[1]);*/                                      \
        XORNEW(new2, new26, new26);                                            \
        XOR(xx1, xx2, xx2);                                                    \
        /*XOR(new1[0],new26[0],new26[0]);                                      \
        XOR(new1[1],new26[1],new26[1]);*/                                      \
        XORNEW(new1, new26, new26);                                            \
        XOR(xx, xx2, xx2);                                                     \
        reductionbymult(new26, xx2, btmp25);                                   \
        XOR(aa[15], key16, btmp1);                                             \
        /*XOR(aa[16],key,btmp17);                                              \
        XOR(aa[17],key2,btmp18);*/                                             \
        XOR(aa[19], key4, btmp2);                                              \
        XOR(aa[20], key, btmp9);                                               \
        XOR(aa[21], key2, btmp17);                                             \
        karatsuba3(btmp1, btmp25, new1, xx, btmp2, btmp18, new2, xx1, btmp9,   \
                   btmp17, new26, xx2);                                        \
        XOR(aa[22], new26[0], new26[0]);                                       \
        /*XOR(new2[0],new26[0],new2[0]);                                       \
        XOR(new2[1],new26[1],new2[1]);*/                                       \
        XORNEW(new2, new26, new2);                                             \
        XOR(xx1, xx2, xx1);                                                    \
        reductionbymult(new2, xx1, btmp2);                                     \
        XOR(aa[23], key8, btmp9);                                              \
        XOR(aa[27], key4, btmp17);                                             \
        XOR(aa[28], key, btmp1);                                               \
        XOR(aa[29], key2, btmp10);                                             \
        karatsuba3(btmp2, btmp9, new2, xx1, btmp26, btmp17, new26, xx2,        \
                   btmp10, btmp1, new10, xx3);                                 \
        XOR(aa[30], new10[0], new10[0]);                                       \
        /*XOR(new10[0],new26[0],new10[0]);                                     \
        XOR(new10[1],new26[1],new10[1]);*/                                     \
        XORNEW(new10, new26, new10);                                           \
        XOR(xx3, xx2, xx3);                                                    \
        /*XOR(new10[0],new2[0],new10[0]);                                      \
        XOR(new10[1],new2[1],new10[1]);*/                                      \
        XORNEW(new10, new2, new10);                                            \
        XOR(xx3, xx1, xx3);                                                    \
        /*XOR(new10[0],new1[0],new10[0]);                                      \
        XOR(new10[1],new1[1],new10[1]);*/                                      \
        XORNEW(new10, new1, new10);                                            \
        XOR(xx3, xx, xx3);                                                     \
        reductionbymult(new10, xx3, dig);                                      \
    })

void partialone(__m128i a[], __m128i key, __m128i *dig) { ASSIGN(*dig, a[0]); }

void partialtwo(__m128i a[], __m128i key, __m128i *dig) {
    __m128i multpart1[2], POLY;

    POLY = _mm_set_epi32(0x0, 0x0, 0x0, 0x00000087);
    karatsuba(a[0], key, multpart1, xx);
    XOR(a[1], multpart1[0], multpart1[0]);
    reductionbymult(multpart1, xx, *dig);
}

void partialthree(__m128i a[], __m128i key, __m128i *dig) {
    __m128i multpart1[2], POLY;

    POLY = _mm_set_epi32(0x0, 0x0, 0x0, 0x00000087);
    XOR(a[0], key, btmp1);
    XOR(a[1], key2, btmp2);
    karatsuba(btmp1, btmp2, multpart1, xx);
    XOR(a[2], multpart1[0], multpart1[0]);
    reductionbymult(multpart1, xx, *dig);
}

void partialfour(__m128i a[], __m128i key, __m128i *dig) {
    __m128i multpart1[2], POLY;

    POLY = _mm_set_epi32(0x0, 0x0, 0x0, 0x00000087);
    XOR(a[0], key, btmp1);
    XOR(a[1], key2, btmp2);
    karatsuba(btmp1, btmp2, multpart1, xx);
    XOR(a[2], multpart1[0], multpart1[0]);
    reductionbymult(multpart1, xx, btmp1);
    XOR(a[3], key4, btmp2);
    mult(btmp1, btmp2, *dig);
}

void partialfive(__m128i a[], __m128i key, __m128i *dig) {

    __m128i multpart1[2], POLY;

    POLY = _mm_set_epi32(0x0, 0x0, 0x0, 0x00000087);
    XOR(a[0], key, btmp1);
    XOR(a[1], key2, btmp2);
    karatsuba(btmp1, btmp2, multpart1, xx);
    XOR(a[2], multpart1[0], multpart1[0]);
    reductionbymult(multpart1, xx, btmp1);
    XOR(a[3], key4, btmp2);
    karatsuba(btmp1, btmp2, multpart1, xx);
    XOR(a[4], multpart1[0], multpart1[0]);
    reductionbymult(multpart1, xx, *dig);
}

void partialsix(__m128i a[], __m128i key, __m128i *dig) {

    __m128i multpart1[2], multpart2[2], POLY;

    POLY = _mm_set_epi32(0x0, 0x0, 0x0, 0x00000087);
    XOR(a[0], key, btmp1);
    XOR(a[1], key2, btmp2);
    karatsuba(btmp1, btmp2, multpart1, xx);
    XOR(a[2], multpart1[0], multpart1[0]);
    reductionbymult(multpart1, xx, btmp1);
    XOR(a[3], key4, btmp2);
    karatsuba2(btmp1, btmp2, multpart1, xx, a[4], key, multpart2, xx1);
    XOR(a[5], multpart2[0], multpart2[0]);
    XORNEW(multpart1, multpart2, multpart1);
    XOR(xx, xx1, xx);
    reductionbymult(multpart1, xx, *dig);
}

void partialseven(__m128i a[], __m128i key, __m128i *dig) {

    __m128i multpart1[2], multpart2[2], POLY;

    POLY = _mm_set_epi32(0x0, 0x0, 0x0, 0x00000087);
    XOR(a[0], key, btmp1);
    XOR(a[1], key2, btmp2);
    karatsuba(btmp1, btmp2, multpart1, xx);
    XOR(a[2], multpart1[0], multpart1[0]);
    reductionbymult(multpart1, xx, btmp1);
    XOR(a[3], key4, btmp2);
    XOR(a[4], key, btmp9);
    XOR(a[5], key2, btmp10);
    karatsuba2(btmp1, btmp2, multpart1, xx, btmp9, btmp10, multpart2, xx1);
    XOR(a[6], multpart2[0], multpart2[0]);
    XORNEW(multpart1, multpart2, multpart1);
    XOR(xx, xx1, xx);
    reductionbymult(multpart1, xx, *dig);
}

void partialeight(__m128i a[], __m128i key, __m128i *dig) {

    __m128i multpart1[2], multpart2[2], POLY;

    POLY = _mm_set_epi32(0x0, 0x0, 0x0, 0x00000087);
    XOR(a[0], key, btmp1);
    XOR(a[1], key2, btmp2);
    karatsuba(btmp1, btmp2, multpart1, xx);
    XOR(a[2], multpart1[0], multpart1[0]);
    reductionbymult(multpart1, xx, btmp1);
    XOR(a[3], key4, btmp2);
    XOR(a[4], key, btmp9);
    XOR(a[5], key2, btmp10);
    karatsuba2(btmp1, btmp2, multpart1, xx, btmp9, btmp10, multpart2, xx1);
    XOR(a[6], multpart2[0], multpart2[0]);
    XORNEW(multpart1, multpart2, multpart1);
    XOR(xx, xx1, xx);
    reductionbymult(multpart1, xx, btmp1);
    XOR(a[7], key8, btmp2);
    mult(btmp1, btmp2, *dig);
}

void partialnine(__m128i a[], __m128i key, __m128i *dig) {

    __m128i multpart1[2], multpart2[2], POLY;

    POLY = _mm_set_epi32(0x0, 0x0, 0x0, 0x00000087);
    XOR(a[0], key, btmp1);
    XOR(a[1], key2, btmp2);
    karatsuba(btmp1, btmp2, multpart1, xx);
    XOR(a[2], multpart1[0], multpart1[0]);
    reductionbymult(multpart1, xx, btmp1);
    XOR(a[3], key4, btmp2);
    XOR(a[4], key, btmp9);
    XOR(a[5], key2, btmp10);
    karatsuba2(btmp1, btmp2, multpart1, xx, btmp9, btmp10, multpart2, xx1);
    XOR(a[6], multpart2[0], multpart2[0]);
    XORNEW(multpart1, multpart2, multpart1);
    XOR(xx, xx1, xx);
    reductionbymult(multpart1, xx, btmp1);
    XOR(a[7], key8, btmp2);
    karatsuba(btmp1, btmp2, multpart1, xx);
    XOR(a[8], multpart1[0], multpart1[0]);
    reductionbymult(multpart1, xx, *dig);
}

void partialten(__m128i a[], __m128i key, __m128i *dig) {

    __m128i multpart1[2], multpart2[2], POLY;

    POLY = _mm_set_epi32(0x0, 0x0, 0x0, 0x00000087);
    XOR(a[0], key, btmp1);
    XOR(a[1], key2, btmp2);
    karatsuba(btmp1, btmp2, multpart1, xx);
    XOR(a[2], multpart1[0], multpart1[0]);
    reductionbymult(multpart1, xx, btmp1);
    XOR(a[3], key4, btmp2);
    XOR(a[4], key, btmp9);
    XOR(a[5], key2, btmp10);
    karatsuba2(btmp1, btmp2, multpart1, xx, btmp9, btmp10, multpart2, xx1);
    XOR(a[6], multpart2[0], multpart2[0]);
    XORNEW(multpart1, multpart2, multpart1);
    XOR(xx, xx1, xx);
    reductionbymult(multpart1, xx, btmp1);
    XOR(a[7], key8, btmp2);
    karatsuba2(btmp1, btmp2, multpart1, xx, a[8], key, multpart2, xx1);
    XOR(a[9], multpart2[0], multpart2[0]);
    XORNEW(multpart1, multpart2, multpart1);
    XOR(xx, xx1, xx);
    reductionbymult(multpart1, xx, *dig);
}

void partialeleven(__m128i a[], __m128i key, __m128i *dig) {

    __m128i multpart1[2], multpart2[2], POLY;

    POLY = _mm_set_epi32(0x0, 0x0, 0x0, 0x00000087);
    XOR(a[0], key, btmp1);
    XOR(a[1], key2, btmp2);
    karatsuba(btmp1, btmp2, multpart1, xx);
    XOR(a[2], multpart1[0], multpart1[0]);
    reductionbymult(multpart1, xx, btmp1);
    XOR(a[3], key4, btmp2);
    XOR(a[4], key, btmp9);
    XOR(a[5], key2, btmp10);
    karatsuba2(btmp1, btmp2, multpart1, xx, btmp9, btmp10, multpart2, xx1);
    XOR(a[6], multpart2[0], multpart2[0]);
    XORNEW(multpart1, multpart2, multpart1);
    XOR(xx, xx1, xx);
    reductionbymult(multpart1, xx, btmp1);
    XOR(a[7], key8, btmp2);
    XOR(a[8], key, btmp9);
    XOR(a[9], key2, btmp10);
    karatsuba2(btmp1, btmp2, multpart1, xx, btmp9, btmp10, multpart2, xx1);
    XOR(a[10], multpart2[0], multpart2[0]);
    XORNEW(multpart1, multpart2, multpart1);
    XOR(xx, xx1, xx);
    reductionbymult(multpart1, xx, *dig);
}

void partialtwelve(__m128i a[], __m128i key, __m128i *dig) {

    __m128i multpart1[2], multpart2[2], POLY;

    POLY = _mm_set_epi32(0x0, 0x0, 0x0, 0x00000087);
    XOR(a[0], key, btmp1);
    XOR(a[1], key2, btmp2);
    karatsuba(btmp1, btmp2, multpart1, xx);
    XOR(a[2], multpart1[0], multpart1[0]);
    reductionbymult(multpart1, xx, btmp1);
    XOR(a[3], key4, btmp2);
    XOR(a[4], key, btmp9);
    XOR(a[5], key2, btmp10);
    karatsuba2(btmp1, btmp2, multpart1, xx, btmp9, btmp10, multpart2, xx1);
    XOR(a[6], multpart2[0], multpart2[0]);
    XORNEW(multpart1, multpart2, multpart1);
    XOR(xx, xx1, xx);
    reductionbymult(multpart1, xx, btmp1);
    XOR(a[7], key8, btmp2);
    XOR(a[8], key, btmp9);
    XOR(a[9], key2, btmp10);
    karatsuba2(btmp1, btmp2, multpart1, xx, btmp9, btmp10, multpart2, xx1);
    XOR(a[10], multpart2[0], multpart2[0]);
    reductionbymult(multpart2, xx1, btmp9);
    XOR(a[11], key4, btmp10);
    karatsuba(btmp9, btmp10, multpart2, xx1);
    XORNEW(multpart1, multpart2, multpart1);
    XOR(xx, xx1, xx);
    reductionbymult(multpart1, xx, *dig);
}

void partialthirteen(__m128i a[], __m128i key, __m128i *dig) {

    __m128i multpart1[2], multpart2[2], POLY;

    POLY = _mm_set_epi32(0x0, 0x0, 0x0, 0x00000087);
    XOR(a[0], key, btmp1);
    XOR(a[1], key2, btmp2);
    karatsuba(btmp1, btmp2, multpart1, xx);
    XOR(a[2], multpart1[0], multpart1[0]);
    reductionbymult(multpart1, xx, btmp1);
    XOR(a[3], key4, btmp2);
    XOR(a[4], key, btmp9);
    XOR(a[5], key2, btmp10);
    karatsuba2(btmp1, btmp2, multpart1, xx, btmp9, btmp10, multpart2, xx1);
    XOR(a[6], multpart2[0], multpart2[0]);
    XORNEW(multpart1, multpart2, multpart1);
    XOR(xx, xx1, xx);
    reductionbymult(multpart1, xx, btmp1);
    XOR(a[7], key8, btmp2);
    XOR(a[8], key, btmp9);
    XOR(a[9], key2, btmp10);
    karatsuba2(btmp1, btmp2, multpart1, xx, btmp9, btmp10, multpart2, xx1);
    XOR(a[10], multpart2[0], multpart2[0]);
    reductionbymult(multpart2, xx1, btmp9);
    XOR(a[11], key4, btmp10);
    karatsuba(btmp9, btmp10, multpart2, xx1);
    XOR(a[12], multpart2[0], multpart2[0]);
    XORNEW(multpart1, multpart2, multpart1);
    XOR(xx, xx1, xx);
    reductionbymult(multpart1, xx, *dig);
}

void partialfourteen(__m128i a[], __m128i key, __m128i *dig) {

    __m128i multpart1[2], multpart2[2], multpart3[2], POLY;

    POLY = _mm_set_epi32(0x0, 0x0, 0x0, 0x00000087);
    XOR(a[0], key, btmp1);
    XOR(a[1], key2, btmp2);
    karatsuba(btmp1, btmp2, multpart1, xx);
    XOR(a[2], multpart1[0], multpart1[0]);
    reductionbymult(multpart1, xx, btmp1);
    XOR(a[3], key4, btmp2);
    XOR(a[4], key, btmp9);
    XOR(a[5], key2, btmp10);
    karatsuba2(btmp1, btmp2, multpart1, xx, btmp9, btmp10, multpart2, xx1);
    XOR(a[6], multpart2[0], multpart2[0]);
    XORNEW(multpart1, multpart2, multpart1);
    XOR(xx, xx1, xx);
    reductionbymult(multpart1, xx, btmp1);
    XOR(a[7], key8, btmp2);
    XOR(a[8], key, btmp9);
    XOR(a[9], key2, btmp10);
    karatsuba2(btmp1, btmp2, multpart1, xx, btmp9, btmp10, multpart2, xx1);
    XOR(a[10], multpart2[0], multpart2[0]);
    reductionbymult(multpart2, xx1, btmp9);
    XOR(a[11], key4, btmp10);
    karatsuba2(btmp9, btmp10, multpart2, xx1, a[12], key, multpart3, xx2);
    XOR(a[13], multpart3[0], multpart3[0]);
    XORNEW(multpart1, multpart2, multpart1);
    XOR(xx, xx1, xx);
    XORNEW(multpart1, multpart3, multpart1);
    XOR(xx, xx2, xx);
    reductionbymult(multpart1, xx, *dig);
}

void partialfifteen(__m128i a[], __m128i key, __m128i *dig) {

    __m128i multpart1[2], multpart2[2], multpart3[2], POLY;

    POLY = _mm_set_epi32(0x0, 0x0, 0x0, 0x00000087);
    XOR(a[0], key, btmp1);
    XOR(a[1], key2, btmp2);
    karatsuba(btmp1, btmp2, multpart1, xx);
    XOR(a[2], multpart1[0], multpart1[0]);
    reductionbymult(multpart1, xx, btmp1);
    XOR(a[3], key4, btmp2);
    XOR(a[4], key, btmp9);
    XOR(a[5], key2, btmp10);
    karatsuba2(btmp1, btmp2, multpart1, xx, btmp9, btmp10, multpart2, xx1);
    XOR(a[6], multpart2[0], multpart2[0]);
    XORNEW(multpart1, multpart2, multpart1);
    XOR(xx, xx1, xx);
    reductionbymult(multpart1, xx, btmp1);
    XOR(a[7], key8, btmp2);
    XOR(a[8], key, btmp9);
    XOR(a[9], key2, btmp10);
    karatsuba2(btmp1, btmp2, multpart1, xx, btmp9, btmp10, multpart2, xx1);
    XOR(a[10], multpart2[0], multpart2[0]);
    reductionbymult(multpart2, xx1, btmp9);
    XOR(a[11], key4, btmp10);
    XOR(a[12], key, btmp1);
    XOR(a[13], key2, btmp2);
    karatsuba2(btmp9, btmp10, multpart2, xx1, btmp1, btmp2, multpart3, xx2);
    XOR(a[14], multpart3[0], multpart3[0]);
    XORNEW(multpart1, multpart2, multpart1);
    XOR(xx, xx1, xx);
    XORNEW(multpart1, multpart3, multpart1);
    XOR(xx, xx2, xx);
    reductionbymult(multpart1, xx, *dig);
}

void partialrest(__m128i a[], __m128i key, int noOfBlock, __m128i *dig,
                 __m128i dig1) {
    __m128i *dig3 = &dig1, tmpdig[2], POLY;
    POLY = _mm_set_epi32(0x0, 0x0, 0x0, 0x00000087);

    partialfifteen(a, key, dig);

    XOR(a[15], key16, btmp1);
    karatsuba(*dig, btmp1, tmpdig, xx);
    reductionbymult(tmpdig, xx, *dig);

    if (noOfBlock == 16)
        return;
    if (noOfBlock == 17) {
        XOR(a[16], tmpdig[0], tmpdig[0]);
        reductionbymult(tmpdig, xx, *dig);
        return;
    }
    switch (noOfBlock) {
    /*case 17:
            partialone(a+16,key,dig3);
            break;*/
    case 18:
        partialtwo(a + 16, key, dig3);
        break;
    case 19:
        partialthree(a + 16, key, dig3);
        break;
    case 20:
        partialfour(a + 16, key, dig3);
        break;
    case 21:
        partialfive(a + 16, key, dig3);
        break;
    case 22:
        partialsix(a + 16, key, dig3);
        break;
    case 23:
        partialseven(a + 16, key, dig3);
        break;
    case 24:
        partialeight(a + 16, key, dig3);
        break;
    case 25:
        partialnine(a + 16, key, dig3);
        break;
    case 26:
        partialten(a + 16, key, dig3);
        break;
    case 27:
        partialeleven(a + 16, key, dig3);
        break;
    case 28:
        partialtwelve(a + 16, key, dig3);
        break;
    case 29:
        partialthirteen(a + 16, key, dig3);
        break;
    case 30:
        partialfourteen(a + 16, key, dig3);
        break;
    }
    XOR(*dig, *dig3, *dig);
}

#define brwpartial(aa, key, noOfBlock, dig)                                    \
    {                                                                          \
        __m128i dig1 = {0}, *dig2 = &dig;                                      \
        switch (noOfBlock) {                                                   \
        case 1:                                                                \
            partialone(aa, key, dig2);                                         \
            break;                                                             \
        case 2:                                                                \
            partialtwo(aa, key, dig2);                                         \
            break;                                                             \
        case 3:                                                                \
            partialthree(aa, key, dig2);                                       \
            break;                                                             \
        case 4:                                                                \
            partialfour(aa, key, dig2);                                        \
            break;                                                             \
        case 5:                                                                \
            partialfive(aa, key, dig2);                                        \
            break;                                                             \
        case 6:                                                                \
            partialsix(aa, key, dig2);                                         \
            break;                                                             \
        case 7:                                                                \
            partialseven(aa, key, dig2);                                       \
            break;                                                             \
        case 8:                                                                \
            partialeight(aa, key, dig2);                                       \
            break;                                                             \
        case 9:                                                                \
            partialnine(aa, key, dig2);                                        \
            break;                                                             \
        case 10:                                                               \
            partialten(aa, key, dig2);                                         \
            break;                                                             \
        case 11:                                                               \
            partialeleven(aa, key, dig2);                                      \
            break;                                                             \
        case 12:                                                               \
            partialtwelve(aa, key, dig2);                                      \
            break;                                                             \
        case 13:                                                               \
            partialthirteen(aa, key, dig2);                                    \
            break;                                                             \
        case 14:                                                               \
            partialfourteen(aa, key, dig2);                                    \
            break;                                                             \
        case 15:                                                               \
            partialfifteen(aa, key, dig2);                                     \
            break;                                                             \
        default:                                                               \
            if (noOfBlock >= 16 && noOfBlock <= 30)                            \
                partialrest(aa, key, noOfBlock, dig2, dig1);                   \
        }                                                                      \
    }

#endif
