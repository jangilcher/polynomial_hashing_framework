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

#include "hash.h"
#include "tigerlake.h"
#include <stdio.h>
#include <stdlib.h>
#define IVSIZE 12

void init_hash(void) {
    if (!haberdashery_aes256gcm_tigerlake_is_supported()) {
        printf("Architecture not supported\n");
        exit(-1);
    }
}

void hash(unsigned char *out, const unsigned char *in, unsigned long long inlen,
          unsigned char *key, const unsigned long long keylen) {
    haberdashery_tigerlake_aes_256_gcm_t aead;
    const unsigned char *nonce =
        key + HABERDASHERY_TIGERLAKE_AES_256_GCM_KEY_LEN;
    unsigned char plaintext[1] = {0};
    unsigned char ciphertext[1] = {0};

    if (!haberdashery_aes256gcm_tigerlake_init(
            &aead, key, HABERDASHERY_TIGERLAKE_AES_256_GCM_KEY_LEN)) {
        printf("could not init haberdashery\n");
        printf("error during haberdashery\n");
        exit(-1);
    }

    if (!haberdashery_aes256gcm_tigerlake_encrypt(
            &aead, nonce, HABERDASHERY_TIGERLAKE_AES_256_GCM_NONCE_LEN, in,
            inlen, plaintext, sizeof(plaintext), ciphertext, sizeof(ciphertext),
            out, HABERDASHERY_TIGERLAKE_AES_256_GCM_TAG_LEN)) {
        return;
    }
}

int hash_verify(unsigned char *out, const unsigned char *in,
                unsigned long long inlen, const unsigned char *key) {
    // TODO
    return 0;
}
