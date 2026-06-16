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

#ifndef __HKM_H
#define __HKM_H
#include <stddef.h>
#include <string.h>

// unsigned long long get_keylength(unsigned long long inlen) {
//     return (((inlen / BLOCKSIZE) + !!(inlen % BLOCKSIZE) + 1) / 2) *
//     BLOCKSIZE;
// }

// Fix for keylength computation
// TODO: double check that this is the right definition
unsigned long long get_keylength(unsigned long long inlen) {
    const unsigned long long num_msg_blocks =
        ((inlen / BLOCKSIZE) + !!(inlen % BLOCKSIZE));
    return get_keylength_LE(
        inlen, (unsigned long long)(((num_msg_blocks / 2) + 1) * KEYSIZE));
}

void HKM(unsigned char *out, const unsigned char *in, unsigned long long inlen,
         const unsigned char *key, unsigned long long keylen);

#endif
