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

#ifndef MULT_H_
#define MULT_H_

#define karatsuba(aa, bb, cc, xx)                                              \
    ({                                                                         \
        xx = _mm_srli_si128(aa, 8);                                            \
        xx = _mm_xor_si128(xx, aa);                                            \
        s = _mm_srli_si128(bb, 8);                                             \
        s = _mm_xor_si128(s, bb);                                              \
        cc[0] = _mm_clmulepi64_si128(aa, bb, 0x00);                            \
        cc[1] = _mm_clmulepi64_si128(aa, bb, 0x11);                            \
        xx = _mm_clmulepi64_si128(xx, s, 0x00);                                \
        s = _mm_xor_si128(cc[0], cc[1]);                                       \
        xx = _mm_xor_si128(xx, s);                                             \
        /*s = _mm_slli_si128 (r, 8);                                           \
        r = _mm_srli_si128 (r, 8);                                             \
        cc[0] = _mm_xor_si128(cc[0],s);                                        \
        cc[1] = _mm_xor_si128(cc[1],r);*/                                      \
    })

#define reductionbymult(cc, xx, dd)                                            \
    ({                                                                         \
        s = _mm_slli_si128(xx, 8);                                             \
        r = _mm_srli_si128(xx, 8);                                             \
        cc[0] = _mm_xor_si128(cc[0], s);                                       \
        cc[1] = _mm_xor_si128(cc[1], r);                                       \
        r = _mm_clmulepi64_si128(cc[1], POLY, 0x01);                           \
        p = _mm_srli_si128(r, 8);                                              \
        p = _mm_xor_si128(p, cc[1]);                                           \
        s = _mm_clmulepi64_si128(p, POLY, 0x00);                               \
        p = _mm_slli_si128(r, 8);                                              \
        dd = _mm_xor_si128(s, p);                                              \
        dd = _mm_xor_si128(dd, cc[0]);                                         \
    })

#define mult(aa, bb, dd)                                                       \
    ({                                                                         \
        __m128i cc[2];                                                         \
        karatsuba(aa, bb, cc, xx);                                             \
        reductionbymult(cc, xx, dd);                                           \
    })

#define karatsuba2(aa, bb, cc, xx, aa1, bb1, cc1, xx1)                         \
    ({                                                                         \
        xx = _mm_srli_si128(aa, 8);                                            \
        xx = _mm_xor_si128(xx, aa);                                            \
        s = _mm_srli_si128(bb, 8);                                             \
        s = _mm_xor_si128(s, bb);                                              \
        xx1 = _mm_srli_si128(aa1, 8);                                          \
        xx1 = _mm_xor_si128(xx1, aa1);                                         \
        s1 = _mm_srli_si128(bb1, 8);                                           \
        s1 = _mm_xor_si128(s1, bb1);                                           \
        cc[0] = _mm_clmulepi64_si128(aa, bb, 0x00);                            \
        cc[1] = _mm_clmulepi64_si128(aa, bb, 0x11);                            \
        xx = _mm_clmulepi64_si128(xx, s, 0x00);                                \
        cc1[0] = _mm_clmulepi64_si128(aa1, bb1, 0x00);                         \
        cc1[1] = _mm_clmulepi64_si128(aa1, bb1, 0x11);                         \
        xx1 = _mm_clmulepi64_si128(xx1, s1, 0x00);                             \
        s = _mm_xor_si128(cc[0], cc[1]);                                       \
        xx = _mm_xor_si128(xx, s);                                             \
        /*s = _mm_slli_si128 (r, 8);                                           \
        r = _mm_srli_si128 (r, 8);                                             \
        cc[0] = _mm_xor_si128(cc[0],s);                                        \
        cc[1] = _mm_xor_si128(cc[1],r);*/                                      \
        s = _mm_xor_si128(cc1[0], cc1[1]);                                     \
        xx1 = _mm_xor_si128(xx1, s);                                           \
        /*s1 = _mm_slli_si128 (r1, 8);                                         \
        r1 = _mm_srli_si128 (r1, 8);                                           \
        cc1[0] = _mm_xor_si128(cc1[0],s1);                                     \
        cc1[1] = _mm_xor_si128(cc1[1],r1);*/                                   \
    })

#define reductionbymult2(cc, xx, dd, cc1, xx1, dd1)                            \
    ({                                                                         \
        s = _mm_slli_si128(xx, 8);                                             \
        r = _mm_srli_si128(xx, 8);                                             \
        cc[0] = _mm_xor_si128(cc[0], s);                                       \
        cc[1] = _mm_xor_si128(cc[1], r);                                       \
        s = _mm_slli_si128(xx1, 8);                                            \
        r = _mm_srli_si128(xx1, 8);                                            \
        cc1[0] = _mm_xor_si128(cc1[0], s);                                     \
        cc1[1] = _mm_xor_si128(cc1[1], r);                                     \
        r = _mm_clmulepi64_si128(cc[1], POLY, 0x01);                           \
        r1 = _mm_clmulepi64_si128(cc1[1], POLY, 0x01);                         \
        p = _mm_srli_si128(r, 8);                                              \
        p1 = _mm_srli_si128(r1, 8);                                            \
        p = _mm_xor_si128(p, cc[1]);                                           \
        p1 = _mm_xor_si128(p1, cc1[1]);                                        \
        s = _mm_clmulepi64_si128(p, POLY, 0x00);                               \
        s1 = _mm_clmulepi64_si128(p1, POLY, 0x00);                             \
        p = _mm_slli_si128(r, 8);                                              \
        dd = _mm_xor_si128(s, p);                                              \
        dd = _mm_xor_si128(dd, cc[0]);                                         \
        p1 = _mm_slli_si128(r1, 8);                                            \
        dd1 = _mm_xor_si128(s1, p1);                                           \
        dd1 = _mm_xor_si128(dd1, cc1[0]);                                      \
    })

#define mult2(aa, bb, dd, aa1, bb1, dd1)                                       \
    ({                                                                         \
        __m128i cc[2], cc1[2];                                                 \
        karatsuba2(aa, bb, cc, xx, aa1, bb1, cc1, xx1);                        \
        reductionbymult2(cc, xx, dd, cc1, xx1, dd1);                           \
    })

#define karatsuba3(aa, bb, cc, xx, aa1, bb1, cc1, xx1, aa2, bb2, cc2, xx2)     \
    ({                                                                         \
        xx = _mm_srli_si128(aa, 8);                                            \
        xx = _mm_xor_si128(xx, aa);                                            \
        s = _mm_srli_si128(bb, 8);                                             \
        s = _mm_xor_si128(s, bb);                                              \
        xx1 = _mm_srli_si128(aa1, 8);                                          \
        xx1 = _mm_xor_si128(xx1, aa1);                                         \
        s1 = _mm_srli_si128(bb1, 8);                                           \
        s1 = _mm_xor_si128(s1, bb1);                                           \
        xx2 = _mm_srli_si128(aa2, 8);                                          \
        xx2 = _mm_xor_si128(xx2, aa2);                                         \
        s2 = _mm_srli_si128(bb2, 8);                                           \
        s2 = _mm_xor_si128(s2, bb2);                                           \
        cc[0] = _mm_clmulepi64_si128(aa, bb, 0x00);                            \
        cc[1] = _mm_clmulepi64_si128(aa, bb, 0x11);                            \
        xx = _mm_clmulepi64_si128(xx, s, 0x00);                                \
        cc1[0] = _mm_clmulepi64_si128(aa1, bb1, 0x00);                         \
        cc1[1] = _mm_clmulepi64_si128(aa1, bb1, 0x11);                         \
        xx1 = _mm_clmulepi64_si128(xx1, s1, 0x00);                             \
        cc2[0] = _mm_clmulepi64_si128(aa2, bb2, 0x00);                         \
        cc2[1] = _mm_clmulepi64_si128(aa2, bb2, 0x11);                         \
        xx2 = _mm_clmulepi64_si128(xx2, s2, 0x00);                             \
        s = _mm_xor_si128(cc[0], cc[1]);                                       \
        xx = _mm_xor_si128(xx, s);                                             \
        /*s = _mm_slli_si128 (r, 8);                                           \
        r = _mm_srli_si128 (r, 8);                                             \
        cc[0] = _mm_xor_si128(cc[0],s);                                        \
        cc[1] = _mm_xor_si128(cc[1],r);*/                                      \
        s = _mm_xor_si128(cc1[0], cc1[1]);                                     \
        xx1 = _mm_xor_si128(xx1, s);                                           \
        /*s1 = _mm_slli_si128 (r1, 8);                                         \
        r1 = _mm_srli_si128 (r1, 8);                                           \
        cc1[0] = _mm_xor_si128(cc1[0],s1);                                     \
        cc1[1] = _mm_xor_si128(cc1[1],r1);*/                                   \
        s = _mm_xor_si128(cc2[0], cc2[1]);                                     \
        xx2 = _mm_xor_si128(xx2, s);                                           \
        /*s2 = _mm_slli_si128 (r2, 8);                                         \
        r2 = _mm_srli_si128 (r2, 8);                                           \
        cc2[0] = _mm_xor_si128(cc2[0],s2);                                     \
        cc2[1] = _mm_xor_si128(cc2[1],r2);*/                                   \
    })

#define reductionbymult3(cc, xx, dd, cc1, xx1, dd1, cc2, xx2, dd2)             \
    ({                                                                         \
        s = _mm_slli_si128(xx, 8);                                             \
        r = _mm_srli_si128(xx, 8);                                             \
        cc[0] = _mm_xor_si128(cc[0], s);                                       \
        cc[1] = _mm_xor_si128(cc[1], r);                                       \
        s = _mm_slli_si128(xx1, 8);                                            \
        r = _mm_srli_si128(xx1, 8);                                            \
        cc1[0] = _mm_xor_si128(cc1[0], s);                                     \
        cc1[1] = _mm_xor_si128(cc1[1], r);                                     \
        s = _mm_slli_si128(xx2, 8);                                            \
        r = _mm_srli_si128(xx2, 8);                                            \
        cc2[0] = _mm_xor_si128(cc2[0], s);                                     \
        cc2[1] = _mm_xor_si128(cc2[1], r);                                     \
        r = _mm_clmulepi64_si128(cc[1], POLY, 0x01);                           \
        r1 = _mm_clmulepi64_si128(cc1[1], POLY, 0x01);                         \
        r2 = _mm_clmulepi64_si128(cc2[1], POLY, 0x01);                         \
        p = _mm_srli_si128(r, 8);                                              \
        p1 = _mm_srli_si128(r1, 8);                                            \
        p2 = _mm_srli_si128(r2, 8);                                            \
        p = _mm_xor_si128(p, cc[1]);                                           \
        p1 = _mm_xor_si128(p1, cc1[1]);                                        \
        p2 = _mm_xor_si128(p2, cc2[1]);                                        \
        s = _mm_clmulepi64_si128(p, POLY, 0x00);                               \
        s1 = _mm_clmulepi64_si128(p1, POLY, 0x00);                             \
        s2 = _mm_clmulepi64_si128(p2, POLY, 0x00);                             \
        p = _mm_slli_si128(r, 8);                                              \
        dd = _mm_xor_si128(s, p);                                              \
        dd = _mm_xor_si128(dd, cc[0]);                                         \
        p1 = _mm_slli_si128(r1, 8);                                            \
        dd1 = _mm_xor_si128(s1, p1);                                           \
        dd1 = _mm_xor_si128(dd1, cc1[0]);                                      \
        p2 = _mm_slli_si128(r2, 8);                                            \
        dd2 = _mm_xor_si128(s2, p2);                                           \
        dd2 = _mm_xor_si128(dd2, cc2[0]);                                      \
    })

#define mult3(aa, bb, dd, aa1, bb1, dd1, aa2, bb2, dd2)                        \
    ({                                                                         \
        __m128i cc[2], cc1[2], cc2[2];                                         \
        karatsuba3(aa, bb, cc, xx, aa1, bb1, cc1, xx1, aa2, bb2, cc2, xx2);    \
        reductionbymult3(cc, xx, dd, cc1, xx1, dd1, cc2, xx2, dd2);            \
    })

#endif
