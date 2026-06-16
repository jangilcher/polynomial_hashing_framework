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
#ifndef _HASH_H_
#define _HASH_H_

#include "brw.h"
#include "choicefield.h"
#include "horner.h"

/*uses brw1 and horner1*/
#if HASHNUMBER == 1

#define HASH2L(msg, key, digest, fullBlocks, remaining, noOfBytes,             \
               hornerLength, full256, result)                                  \
    {                                                                          \
        __m128i g1 = _mm_set_epi32(0x0, 0x0, 0x0, 0x00000425);                 \
        int cntr, ihash;                                                       \
        cntr = 0, ihash = 0;                                                   \
        MULTKEYUPTO32                                                          \
        for (cntr = 0; cntr < fullBlocks;                                      \
             cntr = cntr + 1, ihash = ihash + 31) {                            \
            evalbrwseq((msg + ihash), key, digest[cntr]);                      \
        }                                                                      \
        if (remaining) {                                                       \
            if (full256 == 31) {                                               \
                evalbrwseq((msg + ihash), key, digest[cntr]);                  \
            } else {                                                           \
                brwpartial((msg + ihash), key, full256, digest[cntr]);         \
            }                                                                  \
            cntr++;                                                            \
        }                                                                      \
        horner1(digest, hornerLength, key32, result);                          \
        LENGTHPAD(digest[cntr], noOfBytes * 8);                                \
        mult(result, key2, result);                                            \
        mult(digest[cntr], key, digest[cntr]);                                 \
        XOR(result, digest[cntr], result);                                     \
    }

#endif

/*uses brw2 and horner2*/

#if HASHNUMBER == 2

#define HASH2L(msg, key, digest, fullBlocks, remaining, noOfBytes,             \
               hornerLength, full256, result)                                  \
    {                                                                          \
        __m128i g1 = _mm_set_epi32(0x0, 0x0, 0x0, 0x00000425);                 \
        int cntr, ihash;                                                       \
        cntr = 0, ihash = 0;                                                   \
        MULTKEYUPTO32                                                          \
        for (cntr = 0; cntr < fullBlocks;                                      \
             cntr = cntr + 1, ihash = ihash + 31) {                            \
            evalbrwpar2((msg + ihash), key, digest[cntr]);                     \
        }                                                                      \
        if (remaining) {                                                       \
            if (full256 == 31) {                                               \
                evalbrwpar2((msg + ihash), key, digest[cntr]);                 \
            } else {                                                           \
                brwpartial((msg + ihash), key, full256, digest[cntr]);         \
            }                                                                  \
            cntr++;                                                            \
        }                                                                      \
        horner2(digest, hornerLength, key32, result);                          \
        LENGTHPAD(digest[cntr], noOfBytes * 8);                                \
        mult2(result, key2, result, digest[cntr], key, digest[cntr]);          \
        XOR(result, digest[cntr], result);                                     \
    }
#endif

#endif
