// MIT License
//
// Copyright (c) 2023 Jan Gilcher, Jérôme Govinden
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

#include "randombytes.h"
#include <stdlib.h>
#ifdef USE_OPEN_SSL
#include <openssl/crypto.h>
#include <openssl/opensslv.h>
#include <openssl/rand.h>

int init_lib(void) { return 1; }

void randbytes(unsigned char *const buf, const size_t num) {
    if (RAND_bytes_ex(NULL, buf, num, 256) < 1) {
        exit(-1);
    }
}
#else
#include <sodium.h>
int init_lib(void) { return sodium_init(); }

void randbytes(unsigned char *const buf, const size_t num) {
    return randombytes_buf(buf, num);
}
#endif
