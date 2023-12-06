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

#ifndef __KEY_EXPANSION_H
#define __KEY_EXPANSION_H
#include <sodium.h>
#include <stdint.h>

#define EXPANSION_KEY_SIZE crypto_stream_chacha20_KEYBYTES
#ifndef MAX_RAND_BYTES
#define MAX_RAND_BYTES 512UL
#endif

typedef struct key_expansion_state {
    size_t idx;
    uint8_t next_key[crypto_stream_chacha20_KEYBYTES];
    uint8_t bytes[MAX_RAND_BYTES];
} key_expansion_state_t;

int init(const uint8_t *key);

void get(uint8_t *out, size_t len);

#endif
