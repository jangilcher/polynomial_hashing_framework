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
#ifndef _MULT256_H_
#define _MULT256_H_

#define karatsuba(aa, bb, cc)                                                  \
    ({                                                                         \
        l1[0] = _mm_srli_si128(aa[0], 8);                                      \
        l1[0] = _mm_xor_si128(l1[0], aa[0]);                                   \
        j1[0] = _mm_srli_si128(bb[0], 8);                                      \
        j1[0] = _mm_xor_si128(j1[0], bb[0]);                                   \
        l2[0] = _mm_srli_si128(aa[1], 8);                                      \
        l2[0] = _mm_xor_si128(l2[0], aa[1]);                                   \
        j2[0] = _mm_srli_si128(bb[1], 8);                                      \
        j2[0] = _mm_xor_si128(j2[0], bb[1]);                                   \
        xx[0] = _mm_xor_si128(aa[0], aa[1]);                                   \
        xx[1] = _mm_xor_si128(bb[0], bb[1]);                                   \
        l3[0] = _mm_srli_si128(xx[0], 8);                                      \
        l3[0] = _mm_xor_si128(l3[0], xx[0]);                                   \
        j3[0] = _mm_srli_si128(xx[1], 8);                                      \
        j3[0] = _mm_xor_si128(j3[0], xx[1]);                                   \
        cc[0] = _mm_clmulepi64_si128(aa[0], bb[0], 0x00);                      \
        cc[1] = _mm_clmulepi64_si128(aa[0], bb[0], 0x11);                      \
        l1[0] = _mm_clmulepi64_si128(l1[0], j1[0], 0x00);                      \
        cc[2] = _mm_clmulepi64_si128(aa[1], bb[1], 0x00);                      \
        cc[3] = _mm_clmulepi64_si128(aa[1], bb[1], 0x11);                      \
        l2[0] = _mm_clmulepi64_si128(l2[0], j2[0], 0x00);                      \
        ee[0] = _mm_clmulepi64_si128(xx[0], xx[1], 0x00);                      \
        ee[1] = _mm_clmulepi64_si128(xx[0], xx[1], 0x11);                      \
        l3[0] = _mm_clmulepi64_si128(l3[0], j3[0], 0x00);                      \
        j1[0] = _mm_xor_si128(cc[0], cc[1]);                                   \
        /*cc =_mm256_loadu2_m128i (__m128i const* hiaddr, __m128i const*       \
         * loaddr)*/                                                           \
        l1[0] = _mm_xor_si128(l1[0], j1[0]);                                   \
        j1[0] = _mm_slli_si128(l1[0], 8);                                      \
        l1[0] = _mm_srli_si128(l1[0], 8);                                      \
        cc[0] = _mm_xor_si128(cc[0], j1[0]);                                   \
        cc[1] = _mm_xor_si128(cc[1], l1[0]);                                   \
        j2[0] = _mm_xor_si128(cc[2], cc[3]);                                   \
        /*cc =_mm256_loadu2_m128i (__m128i const* hiaddr, __m128i const*       \
         * loaddr)*/                                                           \
        l2[0] = _mm_xor_si128(l2[0], j2[0]);                                   \
        j2[0] = _mm_slli_si128(l2[0], 8);                                      \
        l2[0] = _mm_srli_si128(l2[0], 8);                                      \
        cc[2] = _mm_xor_si128(cc[2], j2[0]);                                   \
        cc[3] = _mm_xor_si128(cc[3], l2[0]);                                   \
        j3[0] = _mm_xor_si128(ee[0], ee[1]);                                   \
        /*cc =_mm256_loadu2_m128i (__m128i const* hiaddr, __m128i const*       \
         * loaddr)*/                                                           \
        l3[0] = _mm_xor_si128(l3[0], j3[0]);                                   \
        j3[0] = _mm_slli_si128(l3[0], 8);                                      \
        l3[0] = _mm_srli_si128(l3[0], 8);                                      \
        ee[0] = _mm_xor_si128(ee[0], j3[0]);                                   \
        ee[1] = _mm_xor_si128(ee[1], l3[0]);                                   \
        ee[0] = _mm_xor_si128(ee[0], cc[0]);                                   \
        ee[0] = _mm_xor_si128(ee[0], cc[2]);                                   \
        ee[1] = _mm_xor_si128(ee[1], cc[1]);                                   \
        ee[1] = _mm_xor_si128(ee[1], cc[3]);                                   \
        cc[1] = _mm_xor_si128(ee[0], cc[1]);                                   \
        cc[2] = _mm_xor_si128(ee[1], cc[2]);                                   \
    })

#define reductionbymult(cc, dd)                                                \
    ({                                                                         \
        j1[0] = _mm_clmulepi64_si128(cc[3], g1, 0x01);                         \
        j2[0] = _mm_clmulepi64_si128(cc[3], g1, 0x00);                         \
        j3[0] = _mm_slli_si128(j1[0], 8);                                      \
        j1[0] = _mm_srli_si128(j1[0], 8);                                      \
        j1[0] = _mm_xor_si128(cc[2], j1[0]);                                   \
        j2[0] = _mm_xor_si128(j2[0], j3[0]);                                   \
        j3[0] = _mm_clmulepi64_si128(g1, j1[0], 0x00);                         \
        j1[0] = _mm_clmulepi64_si128(g1, j1[0], 0x10);                         \
        l1[0] = _mm_srli_si128(j1[0], 8);                                      \
        j1[0] = _mm_slli_si128(j1[0], 8);                                      \
        dd[0] = _mm_xor_si128(j3[0], j1[0]);                                   \
        dd[0] = _mm_xor_si128(cc[0], dd[0]);                                   \
        dd[1] = _mm_xor_si128(l1[0], j2[0]);                                   \
        dd[1] = _mm_xor_si128(dd[1], cc[1]);                                   \
    })

#define mult(aa, bb, yy)                                                       \
    ({                                                                         \
        __m128i cc[4];                                                         \
        karatsuba(aa, bb, cc);                                                 \
        reductionbymult(cc, yy);                                               \
    })

#define karatsuba2(aa, bb, cc, aa1, bb1, cc1)                                  \
    ({                                                                         \
        l1[0] = _mm_srli_si128(aa[0], 8);                                      \
        l1[0] = _mm_xor_si128(l1[0], aa[0]);                                   \
        j1[0] = _mm_srli_si128(bb[0], 8);                                      \
        j1[0] = _mm_xor_si128(j1[0], bb[0]);                                   \
        l2[0] = _mm_srli_si128(aa[1], 8);                                      \
        l2[0] = _mm_xor_si128(l2[0], aa[1]);                                   \
        j2[0] = _mm_srli_si128(bb[1], 8);                                      \
        j2[0] = _mm_xor_si128(j2[0], bb[1]);                                   \
        xx[0] = _mm_xor_si128(aa[0], aa[1]);                                   \
        xx[1] = _mm_xor_si128(bb[0], bb[1]);                                   \
        l3[0] = _mm_srli_si128(xx[0], 8);                                      \
        l3[0] = _mm_xor_si128(l3[0], xx[0]);                                   \
        j3[0] = _mm_srli_si128(xx[1], 8);                                      \
        j3[0] = _mm_xor_si128(j3[0], xx[1]);                                   \
        l1[1] = _mm_srli_si128(aa1[0], 8);                                     \
        l1[1] = _mm_xor_si128(l1[1], aa1[0]);                                  \
        j1[1] = _mm_srli_si128(bb1[0], 8);                                     \
        j1[1] = _mm_xor_si128(j1[1], bb1[0]);                                  \
        l2[1] = _mm_srli_si128(aa1[1], 8);                                     \
        l2[1] = _mm_xor_si128(l2[1], aa1[1]);                                  \
        j2[1] = _mm_srli_si128(bb1[1], 8);                                     \
        j2[1] = _mm_xor_si128(j2[1], bb1[1]);                                  \
        xx1[0] = _mm_xor_si128(aa1[0], aa1[1]);                                \
        xx1[1] = _mm_xor_si128(bb1[0], bb1[1]);                                \
        l3[1] = _mm_srli_si128(xx1[0], 8);                                     \
        l3[1] = _mm_xor_si128(l3[1], xx1[0]);                                  \
        j3[1] = _mm_srli_si128(xx1[1], 8);                                     \
        j3[1] = _mm_xor_si128(j3[1], xx1[1]);                                  \
        cc[0] = _mm_clmulepi64_si128(aa[0], bb[0], 0x00);                      \
        cc[1] = _mm_clmulepi64_si128(aa[0], bb[0], 0x11);                      \
        l1[0] = _mm_clmulepi64_si128(l1[0], j1[0], 0x00);                      \
        cc[2] = _mm_clmulepi64_si128(aa[1], bb[1], 0x00);                      \
        cc[3] = _mm_clmulepi64_si128(aa[1], bb[1], 0x11);                      \
        l2[0] = _mm_clmulepi64_si128(l2[0], j2[0], 0x00);                      \
        ee[0] = _mm_clmulepi64_si128(xx[0], xx[1], 0x00);                      \
        ee[1] = _mm_clmulepi64_si128(xx[0], xx[1], 0x11);                      \
        l3[0] = _mm_clmulepi64_si128(l3[0], j3[0], 0x00);                      \
        cc1[0] = _mm_clmulepi64_si128(aa1[0], bb1[0], 0x00);                   \
        cc1[1] = _mm_clmulepi64_si128(aa1[0], bb1[0], 0x11);                   \
        l1[1] = _mm_clmulepi64_si128(l1[1], j1[1], 0x00);                      \
        cc1[2] = _mm_clmulepi64_si128(aa1[1], bb1[1], 0x00);                   \
        cc1[3] = _mm_clmulepi64_si128(aa1[1], bb1[1], 0x11);                   \
        l2[1] = _mm_clmulepi64_si128(l2[1], j2[1], 0x00);                      \
        xx[0] = _mm_clmulepi64_si128(xx1[0], xx1[1], 0x00);                    \
        xx[1] = _mm_clmulepi64_si128(xx1[0], xx1[1], 0x11);                    \
        l3[1] = _mm_clmulepi64_si128(l3[1], j3[1], 0x00);                      \
        j1[0] = _mm_xor_si128(cc[0], cc[1]);                                   \
        /*cc =_mm256_loadu2_m128i (__m128i const* hiaddr, __m128i const*       \
         * loaddr)*/                                                           \
        l1[0] = _mm_xor_si128(l1[0], j1[0]);                                   \
        j1[0] = _mm_slli_si128(l1[0], 8);                                      \
        l1[0] = _mm_srli_si128(l1[0], 8);                                      \
        cc[0] = _mm_xor_si128(cc[0], j1[0]);                                   \
        cc[1] = _mm_xor_si128(cc[1], l1[0]);                                   \
        j2[0] = _mm_xor_si128(cc[2], cc[3]);                                   \
        /*cc =_mm256_loadu2_m128i (__m128i const* hiaddr, __m128i const*       \
         * loaddr)*/                                                           \
        l2[0] = _mm_xor_si128(l2[0], j2[0]);                                   \
        j2[0] = _mm_slli_si128(l2[0], 8);                                      \
        l2[0] = _mm_srli_si128(l2[0], 8);                                      \
        cc[2] = _mm_xor_si128(cc[2], j2[0]);                                   \
        cc[3] = _mm_xor_si128(cc[3], l2[0]);                                   \
        j3[0] = _mm_xor_si128(ee[0], ee[1]);                                   \
        /*cc =_mm256_loadu2_m128i (__m128i const* hiaddr, __m128i const*       \
         * loaddr)*/                                                           \
        l3[0] = _mm_xor_si128(l3[0], j3[0]);                                   \
        j3[0] = _mm_slli_si128(l3[0], 8);                                      \
        l3[0] = _mm_srli_si128(l3[0], 8);                                      \
        ee[0] = _mm_xor_si128(ee[0], j3[0]);                                   \
        ee[1] = _mm_xor_si128(ee[1], l3[0]);                                   \
        ee[0] = _mm_xor_si128(ee[0], cc[0]);                                   \
        ee[0] = _mm_xor_si128(ee[0], cc[2]);                                   \
        ee[1] = _mm_xor_si128(ee[1], cc[1]);                                   \
        ee[1] = _mm_xor_si128(ee[1], cc[3]);                                   \
        cc[1] = _mm_xor_si128(ee[0], cc[1]);                                   \
        cc[2] = _mm_xor_si128(ee[1], cc[2]);                                   \
        j1[1] = _mm_xor_si128(cc1[0], cc1[1]);                                 \
        /*cc =_mm256_loadu2_m128i (__m128i const* hiaddr, __m128i const*       \
         * loaddr)*/                                                           \
        l1[1] = _mm_xor_si128(l1[1], j1[1]);                                   \
        j1[1] = _mm_slli_si128(l1[1], 8);                                      \
        l1[1] = _mm_srli_si128(l1[1], 8);                                      \
        cc1[0] = _mm_xor_si128(cc1[0], j1[1]);                                 \
        cc1[1] = _mm_xor_si128(cc1[1], l1[1]);                                 \
        j2[1] = _mm_xor_si128(cc1[2], cc1[3]);                                 \
        /*cc =_mm256_loadu2_m128i (__m128i const* hiaddr, __m128i const*       \
         * loaddr)*/                                                           \
        l2[1] = _mm_xor_si128(l2[1], j2[1]);                                   \
        j2[1] = _mm_slli_si128(l2[1], 8);                                      \
        l2[1] = _mm_srli_si128(l2[1], 8);                                      \
        cc1[2] = _mm_xor_si128(cc1[2], j2[1]);                                 \
        cc1[3] = _mm_xor_si128(cc1[3], l2[1]);                                 \
        j3[1] = _mm_xor_si128(xx[0], xx[1]);                                   \
        /*cc =_mm256_loadu2_m128i (__m128i const* hiaddr, __m128i const*       \
         * loaddr)*/                                                           \
        l3[1] = _mm_xor_si128(l3[1], j3[1]);                                   \
        j3[1] = _mm_slli_si128(l3[1], 8);                                      \
        l3[1] = _mm_srli_si128(l3[1], 8);                                      \
        xx[0] = _mm_xor_si128(xx[0], j3[1]);                                   \
        xx[1] = _mm_xor_si128(xx[1], l3[1]);                                   \
        xx[0] = _mm_xor_si128(xx[0], cc1[0]);                                  \
        xx[0] = _mm_xor_si128(xx[0], cc1[2]);                                  \
        xx[1] = _mm_xor_si128(xx[1], cc1[1]);                                  \
        xx[1] = _mm_xor_si128(xx[1], cc1[3]);                                  \
        cc1[1] = _mm_xor_si128(xx[0], cc1[1]);                                 \
        cc1[2] = _mm_xor_si128(xx[1], cc1[2]);                                 \
    })

#define reductionbymult2(cc, dd, cc1, dd1)                                     \
    ({                                                                         \
        j1[0] = _mm_clmulepi64_si128(cc[3], g1, 0x01);                         \
        j2[0] = _mm_clmulepi64_si128(cc[3], g1, 0x00);                         \
        j1[1] = _mm_clmulepi64_si128(cc1[3], g1, 0x01);                        \
        j2[1] = _mm_clmulepi64_si128(cc1[3], g1, 0x00);                        \
        j3[0] = _mm_slli_si128(j1[0], 8);                                      \
        j1[0] = _mm_srli_si128(j1[0], 8);                                      \
        j1[0] = _mm_xor_si128(cc[2], j1[0]);                                   \
        j2[0] = _mm_xor_si128(j2[0], j3[0]);                                   \
        j3[1] = _mm_slli_si128(j1[1], 8);                                      \
        j1[1] = _mm_srli_si128(j1[1], 8);                                      \
        j1[1] = _mm_xor_si128(cc1[2], j1[1]);                                  \
        j2[1] = _mm_xor_si128(j2[1], j3[1]);                                   \
        j3[0] = _mm_clmulepi64_si128(g1, j1[0], 0x00);                         \
        j1[0] = _mm_clmulepi64_si128(g1, j1[0], 0x10);                         \
        j3[1] = _mm_clmulepi64_si128(g1, j1[1], 0x00);                         \
        j1[1] = _mm_clmulepi64_si128(g1, j1[1], 0x10);                         \
        l1[0] = _mm_srli_si128(j1[0], 8);                                      \
        j1[0] = _mm_slli_si128(j1[0], 8);                                      \
        dd[0] = _mm_xor_si128(j3[0], j1[0]);                                   \
        dd[0] = _mm_xor_si128(cc[0], dd[0]);                                   \
        dd[1] = _mm_xor_si128(l1[0], j2[0]);                                   \
        dd[1] = _mm_xor_si128(dd[1], cc[1]);                                   \
        l1[1] = _mm_srli_si128(j1[1], 8);                                      \
        j1[1] = _mm_slli_si128(j1[1], 8);                                      \
        dd1[0] = _mm_xor_si128(j3[1], j1[1]);                                  \
        dd1[0] = _mm_xor_si128(cc1[0], dd1[0]);                                \
        dd1[1] = _mm_xor_si128(l1[1], j2[1]);                                  \
        dd1[1] = _mm_xor_si128(dd1[1], cc1[1]);                                \
    })

#define mult2(aa, bb, yy, aa1, bb1, yy1)                                       \
    ({                                                                         \
        __m128i cc[4], cc1[4];                                                 \
        karatsuba2(aa, bb, cc, aa1, bb1, cc1);                                 \
        reductionbymult2(cc, yy, cc1, yy1);                                    \
    })

#endif
