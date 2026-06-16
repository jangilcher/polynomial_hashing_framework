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

#include "../../../ref/d2LHash/d2LHash1271/d3/g4/include/d2LHash1271.h"
#include "hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void init_hash(void) {}

void hash(unsigned char *out, const unsigned char *in, unsigned long long inlen,
          unsigned char *key, const unsigned long long keylen) {
    uint64 kp[16];

    key[KEYSIZE - 1] = key[KEYSIZE - 1] & 0x3F;
    unsigned long long len = inlen < 8192ULL ? 8192ULL + 32 : inlen + 32;
    unsigned char *m = (unsigned char *)malloc(len);
    if (!m)
        exit(-1);
    memcpy(m, in, inlen);
    d2LHash1271keypowers(kp, (uint64 *)key);
    d2LHash1271(out, (uchar8 *)m, kp, inlen * 8);
    free(m);
}

int hash_verify(unsigned char *out, const unsigned char *in,
                unsigned long long inlen, const unsigned char *key) {
    // TODO
    return 0;
}
