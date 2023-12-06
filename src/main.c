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

int main(int argc, char *argv[]) {
    if (sodium_init() < 0) {
        return -1;
    }

    size_t input_len = 1024 * 10; // max byte length of input
    size_t input_len_test = input_len;
    unsigned char in[input_len];
    // byte length of output = CRYPTO_HASH = OUTPUTSIZE
    unsigned char out[CRYPTO_HASH] = {0};
    unsigned char key[KEYSIZE];

    // generate key and input randomly
    // randombytes_buf(key, KEYSIZE);
    // randombytes_buf(in, input_len);

    // generate key and input deterministicaly (for testing)
    unsigned char seed[randombytes_SEEDBYTES] = {1};
    randombytes_buf_deterministic(key, KEYSIZE, seed);
    randombytes_buf_deterministic(in, input_len, seed);

    //    printf("Key[ %d ]", KEYSIZE);
    //    for(int i = 0; i < KEYSIZE; i++){
    //        printf("%02x", key[i]);
    //    }
    //    printf("\n");

    // printf("Input length to test=");
    // scanf("%zu", &input_len_test);
    if (argc == 2)
        sscanf(argv[1], "%zu", &input_len_test);

    // computing hash for input of length "input_len_test"
    hash(out, in, input_len_test, key);
    // print hash output
    printf("H[ %zu ] = ", input_len_test);
    for (int i = 0; i < CRYPTO_HASH; i++) {
        printf("%02x", out[i]);
    }
    printf("\n");

    return 0;
}
