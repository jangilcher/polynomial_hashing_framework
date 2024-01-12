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

#include "hash.h"
#include <openssl/err.h>
#include <openssl/evp.h>
#include <stdio.h>
#include <stdlib.h>

void hash(unsigned char *out, const unsigned char *in, unsigned long long inlen,
          const unsigned char *key) {
    EVP_MAC *mac = EVP_MAC_fetch(NULL, "POLY1305", "provider=default");
    EVP_MAC_CTX *ctx = EVP_MAC_CTX_new(mac);

    // Set the key
    if (!EVP_MAC_init(ctx, key, KEYSIZE, NULL))
        goto err;

    // Update the context with the message data
    if (!EVP_MAC_update(ctx, in, inlen))
        goto err;

    // Compute the MAC
    size_t mac_len;
    if (!EVP_MAC_final(ctx, out, &mac_len, CRYPTO_HASH))
        goto err;

    EVP_MAC_CTX_free(ctx);
    EVP_MAC_free(mac);
    return;
err:
    EVP_MAC_CTX_free(ctx);
    EVP_MAC_free(mac);
    exit(1);
}

int hash_verify(unsigned char *out, const unsigned char *in,
                unsigned long long inlen, const unsigned char *key) {
    // TODO
    return 0;
}
