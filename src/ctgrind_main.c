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

#include "../include/hash.h"
#include <sodium.h>
#include <stdio.h>

#define MAX_TEST_MESSAGE_LENGTH 1024
#define CTGRIND_ITERATIONS 100
int main(int argc, char *argv[]) {
    if (sodium_init() < 0) {
        return -1;
    }
    unsigned char out[CRYPTO_HASH] = {0};
    unsigned char key[KEYSIZE];
    unsigned char *message = malloc(MAX_TEST_MESSAGE_LENGTH);
    if (!message) {
        return -1;
    }

    for (int i = 0; i < CTGRIND_ITERATIONS; i++) {
        randombytes_buf(key, KEYSIZE);
        randombytes_buf(message, MAX_TEST_MESSAGE_LENGTH);

        // computing hash for input of length "input_len_test"
        hash(out, message, MAX_TEST_MESSAGE_LENGTH, key);
    }
    free(message);
    return 0;
}
