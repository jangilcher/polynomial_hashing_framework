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
#ifndef _HORNER_H_
#define _HORNER_H_

#include "choicefield.h"

#define horner1(msg2nd, len, key2nd, result)                                   \
    {                                                                          \
        int ih;                                                                \
        ASSIGN(result, msg2nd[0]);                                             \
        for (ih = 1; ih < len; ih++) {                                         \
            mult(result, key2nd, result);                                      \
            XOR(result, msg2nd[ih], result);                                   \
        }                                                                      \
    }

#define horner2(msg2nd, len, key2nd, result)                                   \
    {                                                                          \
        int ih, mod;                                                           \
        mod = len % 2;                                                         \
        if (len == 1) {                                                        \
            ASSIGN(result, msg2nd[0]);                                         \
        } else {                                                               \
            mult(key2nd, key2nd, key2nd2);                                     \
            ASSIGN(nv21, msg2nd[0]);                                           \
            ASSIGN(nv22, msg2nd[1]);                                           \
            switch (mod) {                                                     \
            case 0:                                                            \
                for (ih = 2; ih < len - 1; ih = ih + 2) {                      \
                    mult2(nv21, key2nd2, nv21, nv22, key2nd2, nv22);           \
                    XOR(nv21, msg2nd[ih], nv21);                               \
                    XOR(nv22, msg2nd[ih + 1], nv22);                           \
                }                                                              \
                mult(nv21, key2nd, nv23);                                      \
                XOR(nv23, nv22, result);                                       \
                break;                                                         \
            case 1:                                                            \
                mult(nv21, key2nd2, nv21);                                     \
                XOR(nv21, msg2nd[2], nv21);                                    \
                for (ih = 4; ih < len; ih = ih + 2) {                          \
                    mult2(nv21, key2nd2, nv21, nv22, key2nd2, nv22);           \
                    XOR(nv21, msg2nd[ih], nv21);                               \
                    XOR(nv22, msg2nd[ih - 1], nv22);                           \
                }                                                              \
                mult(nv22, key2nd, nv23);                                      \
                XOR(nv23, nv21, result);                                       \
            }                                                                  \
        }                                                                      \
    }

/*#define horner1(msg2nd,len,key2nd,result) {\
        int i;\
        ASSIGN(result,msg2nd[0]);\
        for(i= 1; i < len; i++)\
        {\
        mult(result,key2nd,result);\
        XOR(result,msg2nd[i],result);}}*/

/*#define horner2(msg2nd,len,key2nd,result) {\
        int i,mod;\
        mod = len % 2;\
        if(len == 1){\
        ASSIGN(result,msg2nd[0]);}\
        else{\
        mult(key2nd,key2nd,key2nd2);\
        ASSIGN(nv21,msg2nd[0]);\
        ASSIGN(nv22,msg2nd[1]);\
        switch(mod){\
        case 0:\
        for(i = 2; i < len-1; i= i+2)\
        {\
        mult2(nv21,key2nd2,nv21,nv22,key2nd2,nv22);\
        XOR(nv21,msg2nd[i],nv21);\
        XOR(nv22,msg2nd[i+1],nv22);}\
        mult(nv21,key2nd,nv23);\
        XOR(nv23,nv22,result);\
        break;\
        case 1:\
        mult(nv21,key2nd2,nv21);\
        XOR(nv21,msg2nd[2],nv21);\
        for(i = 4; i < len; i= i+2)\
        {\
        mult2(nv21,key2nd2,nv21,nv22,key2nd2,nv22);\
        XOR(nv21,msg2nd[i],nv21);\
        XOR(nv22,msg2nd[i-1],nv22);}\
        mult(nv22,key2nd,nv23);\
        XOR(nv23,nv21,result);}}}*/

/*#define horner3(msg2nd,len,key2nd,result) {\
        int i,mod;\
        mod = len %3;\
        if(len == 1){\
        ASSIGN(result,msg2nd[0]);}\
        else if(len == 2){\
        horner2(msg2nd,len,key2nd,result);}\
        else{\
        mult(key2nd,key2nd,key2nd2);\
        mult(key2nd2,key2nd,key2nd3);\
        ASSIGN(nv31,msg2nd[0]);\
        ASSIGN(nv32,msg2nd[1]);\
        ASSIGN(nv33,msg2nd[2]);\
        switch(mod){\
        case 0:\
        for(i = 3; i <= len-3; i= i+3)\
        {\
        mult3(nv31,key2nd3,nv31,nv32,key2nd3,nv32,nv33,key2nd3,nv33);\
        XOR(nv31,msg2nd[i],nv31);\
        XOR(nv32,msg2nd[i+1],nv32);\
        XOR(nv33,msg2nd[i+2],nv33);}\
        mult2(nv31,key2nd2,nv31,nv32,key2nd,nv32);\
        XOR(nv33,nv32,result);\
        XOR(result,nv31,result);\
        break;\
        case 1: \
        mult(nv31,key2nd3,nv31);\
        XOR(nv31,msg2nd[3],nv31);\
        for(i = 4; i <= len -3; i= i+3)\
        {\
        mult3(nv31,key2nd3,nv31,nv32,key2nd3,nv32,nv33,key2nd3,nv33);\
        XOR(nv31,msg2nd[i+2],nv31);\
        XOR(nv32,msg2nd[i],nv32);\
        XOR(nv33,msg2nd[i+1],nv33);}\
        mult2(nv33,key2nd,nv33,nv32,key2nd2,nv32);\
        XOR(nv33,nv31,result);\
        XOR(result,nv32,result);\
        break;\
        case 2:\
        mult2(nv31,key2nd3,nv31,nv32,key2nd3,nv32);\
        XOR(nv31,msg2nd[3],nv31);\
        XOR(nv32,msg2nd[4],nv32);\
        for(i = 5; i <= len-3; i= i+3)\
        {\
        mult3(nv31,key2nd3,nv31,nv32,key2nd3,nv32,nv33,key2nd3,nv33);\
        XOR(nv31,msg2nd[i+1],nv31);\
        XOR(nv32,msg2nd[i+2],nv32);\
        XOR(nv33,msg2nd[i],nv33);}\
        mult2(nv33,key2nd2,nv33,nv31,key2nd,nv31);\
        XOR(nv32,nv31,result);\
        XOR(result,nv33,result);}}}*/

/*#define horner4(msg2nd,len,key2nd,result) {\
        int i,mod;\
        mod = len % 4;\
        if(len == 1){\
        ASSIGN(result,msg2nd[0]);}\
        else if ((len ==2) || (len == 3)){\
        horner3(msg2nd,len,key2nd,result);}\
        else{\
        mult(key2nd,key2nd,key2nd2);\
        mult(key2nd2,key2nd,key2nd3);\
        mult(key2nd3,key2nd,key2nd4);\
        ASSIGN(nv41,msg2nd[0]);\
        ASSIGN(nv42,msg2nd[1]);\
        ASSIGN(nv43,msg2nd[2]);\
        ASSIGN(nv44,msg2nd[3]);\
        switch(mod){\
        case 0:\
        for(i = 4; i <= len-4 ; i= i+4)\
        {\
        mult4(nv41,key2nd4,nv41,nv42,key2nd4,nv42,nv43,key2nd4,nv43,nv44,key2nd4,nv44);\
        XOR(nv41,msg2nd[i],nv41);\
        XOR(nv42,msg2nd[i+1],nv42);\
        XOR(nv43,msg2nd[i+2],nv43);\
        XOR(nv44,msg2nd[i+3],nv44);}\
        mult3(nv41,key2nd3,nv41,nv42,key2nd2,nv42,nv43,key2nd,nv43);\
        XOR(nv43,nv44,result);\
        XOR(result,nv42,result);\
        XOR(result,nv41,result);\
        break;\
        case 1:\
        mult(nv41,key2nd4,nv41);\
        XOR(nv41,msg2nd[4],nv41);\
        for(i = 5; i <= len-4; i= i+4)\
        {\
        mult4(nv41,key2nd4,nv41,nv42,key2nd4,nv42,nv43,key2nd4,nv43,nv44,key2nd4,nv44);\
        XOR(nv41,msg2nd[i+3],nv41);\
        XOR(nv42,msg2nd[i],nv42);\
        XOR(nv43,msg2nd[i+1],nv43);\
        XOR(nv44,msg2nd[i+2],nv44);}\
        mult3(nv42,key2nd3,nv42,nv43,key2nd2,nv43,nv44,key2nd,nv44);\
        XOR(nv43,nv44,result);\
        XOR(result,nv42,result);\
        XOR(result,nv41,result);\
        break;\
        case 2:\
        mult2(nv41,key2nd4,nv41,nv42,key2nd4,nv42);\
        XOR(nv41,msg2nd[4],nv41);\
        XOR(nv42,msg2nd[5],nv42);\
        for(i = 6; i <= len-4; i= i+4)\
        {\
        mult4(nv41,key2nd4,nv41,nv42,key2nd4,nv42,nv43,key2nd4,nv43,nv44,key2nd4,nv44);\
        XOR(nv41,msg2nd[i+2],nv41);\
        XOR(nv42,msg2nd[i+3],nv42);\
        XOR(nv43,msg2nd[i],nv43);\
        XOR(nv44,msg2nd[i+1],nv44);}\
        mult3(nv43,key2nd3,nv43,nv44,key2nd2,nv44,nv41,key2nd,nv41);\
        XOR(nv43,nv44,result);\
        XOR(result,nv42,result);\
        XOR(result,nv41,result);\
        break;\
        case 3:\
        mult3(nv41,key2nd4,nv41,nv42,key2nd4,nv42,nv43,key2nd4,nv43);\
        XOR(nv41,msg2nd[4],nv41);\
        XOR(nv42,msg2nd[5],nv42);\
        XOR(nv43,msg2nd[6],nv43);\
        for(i = 7; i <= len-4; i= i+4)\
        {\
        mult4(nv41,key2nd4,nv41,nv42,key2nd4,nv42,nv43,key2nd4,nv43,nv44,key2nd4,nv44);\
        XOR(nv41,msg2nd[i+1],nv41);\
        XOR(nv42,msg2nd[i+2],nv42);\
        XOR(nv43,msg2nd[i+3],nv43);\
        XOR(nv44,msg2nd[i],nv44);}\
        mult3(nv44,key2nd3,nv44,nv41,key2nd2,nv41,nv42,key2nd,nv42);\
        XOR(nv43,nv44,result);\
        XOR(result,nv42,result);\
        XOR(result,nv41,result);}}}*/

#endif
