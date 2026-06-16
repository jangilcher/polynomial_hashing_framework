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

#include "mult128.h"

#define XOR(var1, var2, var3)                                                  \
    { var3 = _mm_xor_si128(var1, var2); }

#define XORNEW(var1, var2, var3)                                               \
    {                                                                          \
        var3[0] = _mm_xor_si128(var1[0], var2[0]);                             \
        var3[1] = _mm_xor_si128(var1[1], var2[1]);                             \
    }

#define ASSIGN(var1, var2)                                                     \
    { var1 = var2; }

#define LENGTHPAD(var1, var2)                                                  \
    { var1 = _mm_set_epi32(0, 0, 0, var2); }

#define TWOBLOCKS 256
#define THREEBLOCKS 384
#define SEVENBLOCKS 896
#define FIFTEENBLOCKS 1920
#define THIRTYBLOCKS 3840

__m128i key2, key4, key8, key16, key32;
__m128i key2nd2, key2nd3, key2nd4;
__m128i nv21, nv22, nv23, nv31, nv32, nv33, nv34, nv41, nv42, nv43, nv44, nv45,
    nv46, nv47;
__m128i btmp1, btmp2, btmp9, btmp10, btmp17, btmp18, btmp25, btmp26, xx, xx1,
    xx2, xx3;
__m128i r, s, p, r1, s1, p1, r2, s2, p2;

#endif
