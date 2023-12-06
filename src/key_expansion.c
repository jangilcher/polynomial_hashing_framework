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

#include "key_expansion.h"
#include <sodium.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static key_expansion_state_t _state;

static const uint8_t nonce[crypto_stream_chacha20_NONCEBYTES] = {0};

int reseed() {
    uint8_t tmp[MAX_RAND_BYTES + crypto_stream_chacha20_KEYBYTES];
    crypto_stream_chacha20(tmp,
                           MAX_RAND_BYTES + crypto_stream_chacha20_KEYBYTES,
                           nonce, _state.next_key);
    memcpy(_state.next_key, tmp, crypto_stream_chacha20_KEYBYTES);
    memcpy(_state.bytes, tmp + crypto_stream_chacha20_KEYBYTES, MAX_RAND_BYTES);
    _state.idx = 0;
    return 0;
}

int init(const uint8_t *key) {
    uint8_t tmp[MAX_RAND_BYTES + crypto_stream_chacha20_KEYBYTES];
    crypto_stream_chacha20(
        tmp, MAX_RAND_BYTES + crypto_stream_chacha20_KEYBYTES, nonce, key);
    memcpy(_state.next_key, tmp, crypto_stream_chacha20_KEYBYTES);
    memcpy(_state.bytes, tmp + crypto_stream_chacha20_KEYBYTES, MAX_RAND_BYTES);
    _state.idx = 0;
    return 0;
}

void get(uint8_t *out, size_t len) {
    size_t avail = MAX_RAND_BYTES - _state.idx;
    if (len <= avail) {
        memcpy(out, &_state.bytes[_state.idx], len);
    } else {
        memcpy(out, &_state.bytes[_state.idx], avail);
        len -= avail;
        out += avail;
        reseed();
        while (len >= MAX_RAND_BYTES) {
            memcpy(out, _state.bytes, MAX_RAND_BYTES);
            out += MAX_RAND_BYTES;
            len -= MAX_RAND_BYTES;
            reseed();
        }
        memcpy(out, _state.bytes, len);
        _state.idx += len;
    }
}
