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
#ifndef _FIELD_H_
#define _FIELD_H_

#include "mult256.h"

#define XOR(var1, var2, var3)                                                  \
    {                                                                          \
        var3[0] = _mm_xor_si128(var1[0], var2[0]);                             \
        var3[1] = _mm_xor_si128(var1[1], var2[1]);                             \
    }

#define XORNEW(var1, var2, var3)                                               \
    {                                                                          \
        var3[0] = _mm_xor_si128(var1[0], var2[0]);                             \
        var3[1] = _mm_xor_si128(var1[1], var2[1]);                             \
        var3[2] = _mm_xor_si128(var1[2], var2[2]);                             \
        var3[3] = _mm_xor_si128(var1[3], var2[3]);                             \
    }

#define ASSIGN(var1, var2)                                                     \
    {                                                                          \
        var1[0] = var2[0];                                                     \
        var1[1] = var2[1];                                                     \
    }

#define LENGTHPAD(var1, var2)                                                  \
    {                                                                          \
        var1[0] = _mm_set_epi32(0, 0, 0, var2);                                \
        var1[1] = _mm_set_epi32(0, 0, 0, 0);                                   \
    }
#define LENGTHPADLAST(var1, var2, var3)                                        \
    {                                                                          \
        var1[0] = _mm_set_epi32(0, 0, 0, var2);                                \
        var1[1] =                                                              \
            _mm_set_epi8(var3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);   \
    }
#define EMPTYMSG(var1)                                                         \
    {                                                                          \
        var1[0] = _mm_set_epi8(255, 255, 255, 255, 255, 255, 255, 255, 255,    \
                               255, 255, 255, 255, 255, 255, 255);             \
        var1[1] = _mm_set_epi8(255, 255, 255, 255, 255, 255, 255, 255, 255,    \
                               255, 255, 255, 255, 255, 255, 255);             \
    }
#define TWOBLOCKS 512
#define THREEBLOCKS 768
#define SEVENBLOCKS 1792
#define FIFTEENBLOCKS 3840
#define THIRTYBLOCKS 7680

__m128i key2[2], key4[2], key8[2], key16[2], key32[2];
__m128i key2nd2[2], key2nd3[2], key2nd4[2];
__m128i nv21[2], nv22[2], nv23[2], nv31[2], nv32[2], nv33[2], nv34[2], nv41[2],
    nv42[2], nv43[2], nv44[2], nv45[2], nv46[2], nv47[2];
__m128i btmp1[2], btmp2[2], btmp9[2], btmp10[2], btmp17[2], btmp18[2],
    btmp25[2], btmp26[2];
__m128i l1[2], l2[2], l3[2], j1[2], j2[2], j3[2], xx[2], xx1[2], ee[2];

#endif
