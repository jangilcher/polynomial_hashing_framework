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
#ifndef BRW256_H_
#define BRW256_H_

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

#define evalbrwseq(aa, key, dig)                                               \
    ({                                                                         \
        __m128i new1[4], new2[4], new26[4], new10[4];                          \
        XOR(aa[0], key, btmp1);                                                \
        XOR(aa[1], key2, btmp2);                                               \
        /*XOR(aa[4],key,btmp5);                                                \
        XOR(aa[5],key2,btmp6);                                                 \
        XOR(aa[7],key8,btmp8);*/                                               \
        XOR(aa[8], key, btmp9);                                                \
        XOR(aa[9], key2, btmp10);                                              \
        XOR(aa[16], key, btmp17);                                              \
        XOR(aa[17], key2, btmp18);                                             \
        karatsuba(btmp2, btmp1, new1);                                         \
        karatsuba(btmp10, btmp9, new2);                                        \
        karatsuba(btmp18, btmp17, new26);                                      \
        XOR(aa[2], new1, new1);                                                \
        XOR(aa[10], new2, new2);                                               \
        XOR(aa[18], new26, new26);                                             \
        reductionbymult(new1, btmp2);                                          \
        reductionbymult(new2, btmp10);                                         \
        reductionbymult(new26, btmp18);                                        \
        XOR(aa[4], key, btmp1);                                                \
        XOR(aa[5], key2, btmp9);                                               \
        XOR(aa[3], key4, btmp17);                                              \
        XOR(aa[24], key, btmp25);                                              \
        XOR(aa[25], key2, btmp26);                                             \
        karatsuba(btmp1, btmp9, new1);                                         \
        karatsuba(btmp2, btmp17, new2);                                        \
        karatsuba(btmp26, btmp25, new26);                                      \
        XOR(aa[26], new26, new26);                                             \
        reductionbymult(new26, btmp26);                                        \
        XOR(aa[6], new1, new1);                                                \
        XORNEW(new1, new2, new1);                                              \
        /*XOR(new1[1],new2[1],new1[1]);*/                                      \
        reductionbymult(new1, btmp1);                                          \
        XOR(aa[7], key8, btmp9);                                               \
        XOR(aa[11], key4, btmp2);                                              \
        XOR(aa[12], key, btmp17);                                              \
        XOR(aa[13], key2, btmp25);                                             \
        /*XOR(aa[11],key4,btmp12);*/                                           \
        karatsuba(btmp1, btmp9, new1);                                         \
        karatsuba(btmp2, btmp10, new2);                                        \
        karatsuba(btmp25, btmp17, new26);                                      \
        XOR(aa[14], new26, new26);                                             \
        XORNEW(new2, new26, new26);                                            \
        /*XOR(new2[1],new26[1],new26[1]);*/                                    \
        XORNEW(new1, new26, new26);                                            \
        /*XOR(new1[1],new26[1],new26[1]);*/                                    \
        reductionbymult(new26, btmp25);                                        \
        XOR(aa[15], key16, btmp1);                                             \
        /*XOR(aa[16],key,btmp17);                                              \
        XOR(aa[17],key2,btmp18);*/                                             \
        XOR(aa[19], key4, btmp2);                                              \
        XOR(aa[20], key, btmp9);                                               \
        XOR(aa[21], key2, btmp17);                                             \
        karatsuba(btmp1, btmp25, new1);                                        \
        karatsuba(btmp2, btmp18, new2);                                        \
        karatsuba(btmp9, btmp17, new26);                                       \
        XOR(aa[22], new26, new26);                                             \
        XORNEW(new2, new26, new2);                                             \
        /*XOR(new2[1],new26[1],new2[1]);*/                                     \
        reductionbymult(new2, btmp2);                                          \
        XOR(aa[23], key8, btmp9);                                              \
        XOR(aa[27], key4, btmp17);                                             \
        XOR(aa[28], key, btmp1);                                               \
        XOR(aa[29], key2, btmp10);                                             \
        karatsuba(btmp2, btmp9, new2);                                         \
        karatsuba(btmp26, btmp17, new26);                                      \
        karatsuba(btmp10, btmp1, new10);                                       \
        XOR(aa[30], new10, new10);                                             \
        XORNEW(new10, new26, new10);                                           \
        /*XOR(new10[1],new26[1],new10[1]);*/                                   \
        XORNEW(new10, new2, new10);                                            \
        /*XOR(new10[1],new2[1],new10[1]);*/                                    \
        XORNEW(new10, new1, new10);                                            \
        /*XOR(new10[1],new1[1],new10[1]);*/                                    \
        reductionbymult(new10, dig);                                           \
    })

void partialone(__m128i a[][2], __m128i key[2], __m128i dig[2]) {
    ASSIGN(dig, a[0]);
}

void partialtwo(__m128i a[][2], __m128i key[2], __m128i dig[2]) {
    __m128i multpart1[4], g1;

    g1 = _mm_set_epi32(0x0, 0x0, 0x0, 0x00000425);
    karatsuba(a[0], key, multpart1);
    XOR(a[1], multpart1, multpart1);
    reductionbymult(multpart1, dig);
}

void partialthree(__m128i a[][2], __m128i key[2], __m128i dig[2]) {
    __m128i multpart1[4], g1;
    g1 = _mm_set_epi32(0x0, 0x0, 0x0, 0x00000425);
    XOR(a[0], key, btmp1);
    XOR(a[1], key2, btmp2);
    karatsuba(btmp1, btmp2, multpart1);
    XOR(a[2], multpart1, multpart1);
    reductionbymult(multpart1, dig);
}

void partialfour(__m128i a[][2], __m128i key[2], __m128i dig[2]) {
    __m128i multpart1[4], g1;
    g1 = _mm_set_epi32(0x0, 0x0, 0x0, 0x00000425);
    XOR(a[0], key, btmp1);
    XOR(a[1], key2, btmp2);
    karatsuba(btmp1, btmp2, multpart1);
    XOR(a[2], multpart1, multpart1);
    reductionbymult(multpart1, btmp1);
    XOR(a[3], key4, btmp2);
    mult(btmp1, btmp2, dig);
}

void partialfive(__m128i a[][2], __m128i key[2], __m128i dig[2]) {

    __m128i multpart1[4], g1;
    g1 = _mm_set_epi32(0x0, 0x0, 0x0, 0x00000425);
    XOR(a[0], key, btmp1);
    XOR(a[1], key2, btmp2);
    karatsuba(btmp1, btmp2, multpart1);
    XOR(a[2], multpart1, multpart1);
    reductionbymult(multpart1, btmp1);
    XOR(a[3], key4, btmp2);
    karatsuba(btmp1, btmp2, multpart1);
    XOR(a[4], multpart1, multpart1);
    reductionbymult(multpart1, dig);
}

void partialsix(__m128i a[][2], __m128i key[2], __m128i dig[2]) {

    __m128i multpart1[4], multpart2[4], g1;
    g1 = _mm_set_epi32(0x0, 0x0, 0x0, 0x00000425);
    XOR(a[0], key, btmp1);
    XOR(a[1], key2, btmp2);
    karatsuba(btmp1, btmp2, multpart1);
    XOR(a[2], multpart1, multpart1);
    reductionbymult(multpart1, btmp1);
    XOR(a[3], key4, btmp2);
    karatsuba2(btmp1, btmp2, multpart1, a[4], key, multpart2);
    XOR(a[5], multpart2, multpart2);
    XORNEW(multpart1, multpart2, multpart1);
    reductionbymult(multpart1, dig);
}

void partialseven(__m128i a[][2], __m128i key[2], __m128i dig[2]) {

    __m128i multpart1[4], multpart2[4], g1;
    g1 = _mm_set_epi32(0x0, 0x0, 0x0, 0x00000425);
    XOR(a[0], key, btmp1);
    XOR(a[1], key2, btmp2);
    karatsuba(btmp1, btmp2, multpart1);
    XOR(a[2], multpart1, multpart1);
    reductionbymult(multpart1, btmp1);
    XOR(a[3], key4, btmp2);
    XOR(a[4], key, btmp9);
    XOR(a[5], key2, btmp10);
    karatsuba2(btmp1, btmp2, multpart1, btmp9, btmp10, multpart2);
    XOR(a[6], multpart2, multpart2);
    XORNEW(multpart1, multpart2, multpart1);
    reductionbymult(multpart1, dig);
}

void partialeight(__m128i a[][2], __m128i key[2], __m128i dig[2]) {

    __m128i multpart1[4], multpart2[4], g1;
    g1 = _mm_set_epi32(0x0, 0x0, 0x0, 0x00000425);
    XOR(a[0], key, btmp1);
    XOR(a[1], key2, btmp2);
    karatsuba(btmp1, btmp2, multpart1);
    XOR(a[2], multpart1, multpart1);
    reductionbymult(multpart1, btmp1);
    XOR(a[3], key4, btmp2);
    XOR(a[4], key, btmp9);
    XOR(a[5], key2, btmp10);
    karatsuba2(btmp1, btmp2, multpart1, btmp9, btmp10, multpart2);
    XOR(a[6], multpart2, multpart2);
    XORNEW(multpart1, multpart2, multpart1);
    reductionbymult(multpart1, btmp1);
    XOR(a[7], key8, btmp2);
    mult(btmp1, btmp2, dig);
}

void partialnine(__m128i a[][2], __m128i key[2], __m128i dig[2]) {

    __m128i multpart1[4], multpart2[4], g1;
    g1 = _mm_set_epi32(0x0, 0x0, 0x0, 0x00000425);
    XOR(a[0], key, btmp1);
    XOR(a[1], key2, btmp2);
    karatsuba(btmp1, btmp2, multpart1);
    XOR(a[2], multpart1, multpart1);
    reductionbymult(multpart1, btmp1);
    XOR(a[3], key4, btmp2);
    XOR(a[4], key, btmp9);
    XOR(a[5], key2, btmp10);
    karatsuba2(btmp1, btmp2, multpart1, btmp9, btmp10, multpart2);
    XOR(a[6], multpart2, multpart2);
    XORNEW(multpart1, multpart2, multpart1);
    reductionbymult(multpart1, btmp1);
    XOR(a[7], key8, btmp2);
    karatsuba(btmp1, btmp2, multpart1);
    XOR(a[8], multpart1, multpart1);
    reductionbymult(multpart1, dig);
}

void partialten(__m128i a[][2], __m128i key[2], __m128i dig[2]) {

    __m128i multpart1[4], multpart2[4], g1;
    g1 = _mm_set_epi32(0x0, 0x0, 0x0, 0x00000425);
    XOR(a[0], key, btmp1);
    XOR(a[1], key2, btmp2);
    karatsuba(btmp1, btmp2, multpart1);
    XOR(a[2], multpart1, multpart1);
    reductionbymult(multpart1, btmp1);
    XOR(a[3], key4, btmp2);
    XOR(a[4], key, btmp9);
    XOR(a[5], key2, btmp10);
    karatsuba2(btmp1, btmp2, multpart1, btmp9, btmp10, multpart2);
    XOR(a[6], multpart2, multpart2);
    XORNEW(multpart1, multpart2, multpart1);
    reductionbymult(multpart1, btmp1);
    XOR(a[7], key8, btmp2);
    karatsuba2(btmp1, btmp2, multpart1, a[8], key, multpart2);
    XOR(a[9], multpart2, multpart2);
    XORNEW(multpart1, multpart2, multpart1);
    reductionbymult(multpart1, dig);
}

void partialeleven(__m128i a[][2], __m128i key[2], __m128i dig[2]) {

    __m128i multpart1[4], multpart2[4], g1;
    g1 = _mm_set_epi32(0x0, 0x0, 0x0, 0x00000425);
    XOR(a[0], key, btmp1);
    XOR(a[1], key2, btmp2);
    karatsuba(btmp1, btmp2, multpart1);
    XOR(a[2], multpart1, multpart1);
    reductionbymult(multpart1, btmp1);
    XOR(a[3], key4, btmp2);
    XOR(a[4], key, btmp9);
    XOR(a[5], key2, btmp10);
    karatsuba2(btmp1, btmp2, multpart1, btmp9, btmp10, multpart2);
    XOR(a[6], multpart2, multpart2);
    XORNEW(multpart1, multpart2, multpart1);
    reductionbymult(multpart1, btmp1);
    XOR(a[7], key8, btmp2);
    XOR(a[8], key, btmp9);
    XOR(a[9], key2, btmp10);
    karatsuba2(btmp1, btmp2, multpart1, btmp9, btmp10, multpart2);
    XOR(a[10], multpart2, multpart2);
    XORNEW(multpart1, multpart2, multpart1);
    reductionbymult(multpart1, dig);
}

void partialtwelve(__m128i a[][2], __m128i key[2], __m128i dig[2]) {

    __m128i multpart1[4], multpart2[4], g1;
    g1 = _mm_set_epi32(0x0, 0x0, 0x0, 0x00000425);
    XOR(a[0], key, btmp1);
    XOR(a[1], key2, btmp2);
    karatsuba(btmp1, btmp2, multpart1);
    XOR(a[2], multpart1, multpart1);
    reductionbymult(multpart1, btmp1);
    XOR(a[3], key4, btmp2);
    XOR(a[4], key, btmp9);
    XOR(a[5], key2, btmp10);
    karatsuba2(btmp1, btmp2, multpart1, btmp9, btmp10, multpart2);
    XOR(a[6], multpart2, multpart2);
    XORNEW(multpart1, multpart2, multpart1);
    reductionbymult(multpart1, btmp1);
    XOR(a[7], key8, btmp2);
    XOR(a[8], key, btmp9);
    XOR(a[9], key2, btmp10);
    karatsuba2(btmp1, btmp2, multpart1, btmp9, btmp10, multpart2);
    XOR(a[10], multpart2, multpart2);
    reductionbymult(multpart2, btmp9);
    XOR(a[11], key4, btmp10);
    karatsuba(btmp9, btmp10, multpart2);
    XORNEW(multpart1, multpart2, multpart1);
    reductionbymult(multpart1, dig);
}

void partialthirteen(__m128i a[][2], __m128i key[2], __m128i dig[2]) {

    __m128i multpart1[4], multpart2[4], g1;
    g1 = _mm_set_epi32(0x0, 0x0, 0x0, 0x00000425);
    XOR(a[0], key, btmp1);
    XOR(a[1], key2, btmp2);
    karatsuba(btmp1, btmp2, multpart1);
    XOR(a[2], multpart1, multpart1);
    reductionbymult(multpart1, btmp1);
    XOR(a[3], key4, btmp2);
    XOR(a[4], key, btmp9);
    XOR(a[5], key2, btmp10);
    karatsuba2(btmp1, btmp2, multpart1, btmp9, btmp10, multpart2);
    XOR(a[6], multpart2, multpart2);
    XORNEW(multpart1, multpart2, multpart1);
    reductionbymult(multpart1, btmp1);
    XOR(a[7], key8, btmp2);
    XOR(a[8], key, btmp9);
    XOR(a[9], key2, btmp10);
    karatsuba2(btmp1, btmp2, multpart1, btmp9, btmp10, multpart2);
    XOR(a[10], multpart2, multpart2);
    reductionbymult(multpart2, btmp9);
    XOR(a[11], key4, btmp10);
    karatsuba(btmp9, btmp10, multpart2);
    XOR(a[12], multpart2, multpart2);
    XORNEW(multpart1, multpart2, multpart1);
    reductionbymult(multpart1, dig);
}

void partialfourteen(__m128i a[][2], __m128i key[2], __m128i dig[2]) {

    __m128i multpart1[4], multpart2[4], multpart3[4], g1;
    g1 = _mm_set_epi32(0x0, 0x0, 0x0, 0x00000425);
    XOR(a[0], key, btmp1);
    XOR(a[1], key2, btmp2);
    karatsuba(btmp1, btmp2, multpart1);
    XOR(a[2], multpart1, multpart1);
    reductionbymult(multpart1, btmp1);
    XOR(a[3], key4, btmp2);
    XOR(a[4], key, btmp9);
    XOR(a[5], key2, btmp10);
    karatsuba2(btmp1, btmp2, multpart1, btmp9, btmp10, multpart2);
    XOR(a[6], multpart2, multpart2);
    XORNEW(multpart1, multpart2, multpart1);
    reductionbymult(multpart1, btmp1);
    XOR(a[7], key8, btmp2);
    XOR(a[8], key, btmp9);
    XOR(a[9], key2, btmp10);
    karatsuba2(btmp1, btmp2, multpart1, btmp9, btmp10, multpart2);
    XOR(a[10], multpart2, multpart2);
    reductionbymult(multpart2, btmp9);
    XOR(a[11], key4, btmp10);
    karatsuba2(btmp9, btmp10, multpart2, a[12], key, multpart3);
    XOR(a[13], multpart3, multpart3);
    XORNEW(multpart1, multpart2, multpart1);
    XORNEW(multpart1, multpart3, multpart1);
    reductionbymult(multpart1, dig);
}

void partialfifteen(__m128i a[][2], __m128i key[2], __m128i dig[2]) {

    __m128i multpart1[4], multpart2[4], multpart3[4], g1;
    g1 = _mm_set_epi32(0x0, 0x0, 0x0, 0x00000425);
    XOR(a[0], key, btmp1);
    XOR(a[1], key2, btmp2);
    karatsuba(btmp1, btmp2, multpart1);
    XOR(a[2], multpart1, multpart1);
    reductionbymult(multpart1, btmp1);
    XOR(a[3], key4, btmp2);
    XOR(a[4], key, btmp9);
    XOR(a[5], key2, btmp10);
    karatsuba2(btmp1, btmp2, multpart1, btmp9, btmp10, multpart2);
    XOR(a[6], multpart2, multpart2);
    XORNEW(multpart1, multpart2, multpart1);
    reductionbymult(multpart1, btmp1);
    XOR(a[7], key8, btmp2);
    XOR(a[8], key, btmp9);
    XOR(a[9], key2, btmp10);
    karatsuba2(btmp1, btmp2, multpart1, btmp9, btmp10, multpart2);
    XOR(a[10], multpart2, multpart2);
    reductionbymult(multpart2, btmp9);
    XOR(a[11], key4, btmp10);
    XOR(a[12], key, btmp1);
    XOR(a[13], key2, btmp2);
    karatsuba2(btmp9, btmp10, multpart2, btmp1, btmp2, multpart3);
    XOR(a[14], multpart3, multpart3);
    XORNEW(multpart1, multpart2, multpart1);
    XORNEW(multpart1, multpart3, multpart1);
    reductionbymult(multpart1, dig);
}

void partialrest(__m128i a[][2], __m128i key[2], int noOfBlock, __m128i dig[2],
                 __m128i dig1[2]) {

    __m128i tmpdig[4], g1;
    g1 = _mm_set_epi32(0x0, 0x0, 0x0, 0x00000425);

    partialfifteen(a, key, dig);

    XOR(a[15], key16, btmp1);

    karatsuba(dig, btmp1, tmpdig);
    reductionbymult(tmpdig, dig);
    if (noOfBlock == 16)
        return;
    if (noOfBlock == 17) {
        XOR(a[16], tmpdig, tmpdig);
        reductionbymult(tmpdig, dig);
        return;
    }
    switch (noOfBlock) {
    /*case 17:
            partialone(a[16],key,dig1);
            break;*/
    case 18:
        partialtwo(a[16], key, dig1);
        break;
    case 19:
        partialthree(a[16], key, dig1);
        break;
    case 20:
        partialfour(a[16], key, dig1);
        break;
    case 21:
        partialfive(a[16], key, dig1);
        break;
    case 22:
        partialsix(a[16], key, dig1);
        break;
    case 23:
        partialseven(a[16], key, dig1);
        break;
    case 24:
        partialeight(a[16], key, dig1);
        break;
    case 25:
        partialnine(a[16], key, dig1);
        break;
    case 26:
        partialten(a[16], key, dig1);
        break;
    case 27:
        partialeleven(a[16], key, dig1);
        break;
    case 28:
        partialtwelve(a[16], key, dig1);
        break;
    case 29:
        partialthirteen(a[16], key, dig1);
        break;
    case 30:
        partialfourteen(a[16], key, dig1);
        break;
    }
    XOR(dig, dig1, dig);
}

#define brwpartial(aa, key, noOfBlock, dig)                                    \
    {                                                                          \
        __m128i dig1[2];                                                       \
        switch (noOfBlock) {                                                   \
        case 1:                                                                \
            partialone(aa, key, dig);                                          \
            break;                                                             \
        case 2:                                                                \
            partialtwo(aa, key, dig);                                          \
            break;                                                             \
        case 3:                                                                \
            partialthree(aa, key, dig);                                        \
            break;                                                             \
        case 4:                                                                \
            partialfour(aa, key, dig);                                         \
            break;                                                             \
        case 5:                                                                \
            partialfive(aa, key, dig);                                         \
            break;                                                             \
        case 6:                                                                \
            partialsix(aa, key, dig);                                          \
            break;                                                             \
        case 7:                                                                \
            partialseven(aa, key, dig);                                        \
            break;                                                             \
        case 8:                                                                \
            partialeight(aa, key, dig);                                        \
            break;                                                             \
        case 9:                                                                \
            partialnine(aa, key, dig);                                         \
            break;                                                             \
        case 10:                                                               \
            partialten(aa, key, dig);                                          \
            break;                                                             \
        case 11:                                                               \
            partialeleven(aa, key, dig);                                       \
            break;                                                             \
        case 12:                                                               \
            partialtwelve(aa, key, dig);                                       \
            break;                                                             \
        case 13:                                                               \
            partialthirteen(aa, key, dig);                                     \
            break;                                                             \
        case 14:                                                               \
            partialfourteen(aa, key, dig);                                     \
            break;                                                             \
        case 15:                                                               \
            partialfifteen(aa, key, dig);                                      \
            break;                                                             \
        default:                                                               \
            if (noOfBlock >= 16 && noOfBlock <= 30)                            \
                partialrest(aa, key, noOfBlock, dig, dig1);                    \
        }                                                                      \
    }

#endif
