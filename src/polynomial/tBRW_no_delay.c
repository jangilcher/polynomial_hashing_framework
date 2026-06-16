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

#include "../field_arithmetic/field_arithmetic.h"
#include "../transform/transform.h"
#include "boost/preprocessor/arithmetic/add.hpp"
#include "boost/preprocessor/arithmetic/mul.hpp"
#include "boost/preprocessor/arithmetic/sub.hpp"
#include "boost/preprocessor/repetition/repeat.hpp"
#include "boost/preprocessor/repetition/repeat_from_to.hpp"
#include <stddef.h>
#include <string.h>

#if EXPLICIT_LENGTH_ENCODE
#include "../length_encoding.h"
#endif
#if defined(INNERPOLY_H) && defined(INNERPOLY)
#include INNERPOLY_H
#endif

#define DEBUGPRINTF(...) // printf(__VA_ARGS__)
#define DEBUGPRINT_FIELD_ELEM_ARRAY_ELEM(arr, idx)                             \
    DEBUGPRINTF(#arr "[%zu].val[0]: %" PRIx64 "\n", idx, arr[idx].val[0]);     \
    DEBUGPRINTF(#arr "[%zu].val[1]: %" PRIx64 "\n", idx, arr[idx].val[1]);

#define DEBUGPRINT_FIELD_ELEM(el)                                              \
    DEBUGPRINTF(#el ".val[0]: %" PRIx64 "\n", el.val[0]);                      \
    DEBUGPRINTF(#el ".val[1]: %" PRIx64 "\n", el.val[1]);

// If x > 0: Returns exponent of highest power of 2 that divides x
// UNDEFINED if x == 0
static inline int highest2Power(unsigned long long x) {
    return __builtin_ctzll(x);
}

// If x > 0: Returns exponent of highest power of 2 smaller than x
// UNDEFINED if x == 0
static inline int floor_log2(unsigned long long x) {
    return 63 - __builtin_clzll(x);
}

void tBRW_no_delay(unsigned char *out, const unsigned char *in,
                   unsigned long long inlen, const unsigned char *key,
                   unsigned long long keylen) {
#if EXPLICIT_LENGTH_ENCODE
    const unsigned long long msglen = inlen;
#endif
    unsigned char tag_packed[BUFFSIZE] = {0};
    if (inlen == 0) {
#if EXPLICIT_LENGTH_ENCODE
        field_elem_t acc = {0};
        LENGTH_ENCODING(&acc, &acc, key, keylen, msglen);
        reduce(&acc, &acc);
        pack_field_elem((baseint_t *)tag_packed, &acc);
        transform_field_elem(out, OUTPUTSIZE, tag_packed, BUFFSIZE);
#else
        memset(out, 0, OUTPUTSIZE);
#endif
        return;
    }

    if (inlen <= SUPERBLOCKSIZE) {
        field_elem_t acc = {0};
        INNERPOLY(&acc, in, inlen, key, 1);
#if EXPLICIT_LENGTH_ENCODE
        LENGTH_ENCODING(&acc, &acc, key, keylen, msglen);
        reduce(&acc, &acc);
#endif
        pack_field_elem((baseint_t *)tag_packed, &acc);
    } else if (inlen <= SUPERBLOCKSIZE + BLOCKSIZE) {
        field_elem_t acc = {0};
        field_elem_t t = {0};
        field_elem_t k = {0};
        INNERPOLY(&acc, in, SUPERBLOCKSIZE, key, 0);
        in += SUPERBLOCKSIZE;
        inlen -= SUPERBLOCKSIZE;
        unpack_and_encode_last_field_elem(&t, (baseint_t *)in, inlen);
        in += BLOCKSIZE;
        inlen -= BLOCKSIZE;
        unpack_and_encode_key(&k, (baseint_t *)(key + SUPERKEYSIZE));
        field_add_reduce(&t, &t, &k);
        field_mul_reduce(&acc, &acc, &t);
#if EXPLICIT_LENGTH_ENCODE
        LENGTH_ENCODING(&acc, &acc, key, keylen, msglen);
        reduce(&acc, &acc);
#endif
        pack_field_elem((baseint_t *)tag_packed, &acc);
    } else {
        uint64_t n =
            (((inlen / BLOCKSIZE) + !!(inlen % BLOCKSIZE)) * BLOCKSIZE) /
            (SUPERBLOCKSIZE + BLOCKSIZE);
        uint64_t s = floor_log2(n);
        field_elem_t *acc = calloc(s + 2, sizeof(field_elem_t));
        if (!acc)
            exit(-1);
        field_elem_t t = {0};
        field_elem_t k = {0};

        int i = 0;
        for (i = 0; i < n - 1; i++) {
            INNERPOLY(acc, in, SUPERBLOCKSIZE, key, (inlen == SUPERBLOCKSIZE));
            in += SUPERBLOCKSIZE;
            inlen -= SUPERBLOCKSIZE;

            unpack_and_encode_field_elem(&t, (baseint_t *)in);
            in += BLOCKSIZE;
            inlen -= BLOCKSIZE;

            s = highest2Power(i + 1);
            unpack_and_encode_key(
                &k, (baseint_t *)(key + SUPERKEYSIZE + ((s * KEYSIZE))));

            field_add_reduce(&t, &t, &k);

            acc[s + 1] = acc[0];

            for (int j = 1; j < s + 1; j++) {
                field_add_reduce(acc + s + 1, acc + s + 1, acc + j);
            }
            field_mul_reduce(acc + s + 1, acc + s + 1, &t);
        }
        if (inlen > 0) {
            if (inlen > SUPERBLOCKSIZE + BLOCKSIZE) {
                INNERPOLY(acc, in, SUPERBLOCKSIZE, key,
                          (inlen == SUPERBLOCKSIZE));
                in += SUPERBLOCKSIZE;
                inlen -= SUPERBLOCKSIZE;

                unpack_and_encode_field_elem(&t, (baseint_t *)in);
                in += BLOCKSIZE;
                inlen -= BLOCKSIZE;

                s = highest2Power(i + 1);
                unpack_and_encode_key(
                    &k, (baseint_t *)(key + SUPERKEYSIZE + ((s * KEYSIZE))));

                field_add_reduce(&t, &t, &k);

                acc[s + 1] = acc[0];

                for (int j = 1; j < s + 1; j++) {
                    field_add_reduce(acc + s + 1, acc + s + 1, acc + j);
                }
                field_mul_reduce(acc + s + 1, acc + s + 1, &t);
                i++;
            }
            if (inlen > SUPERBLOCKSIZE) {
                INNERPOLY(acc, in, SUPERBLOCKSIZE, key, 0);
                in += SUPERBLOCKSIZE;
                inlen -= SUPERBLOCKSIZE;

                unpack_and_encode_last_field_elem(&t, (baseint_t *)in, inlen);
                in += BLOCKSIZE;
                inlen -= BLOCKSIZE;

                s = highest2Power(i + 1);
                unpack_and_encode_key(
                    &k, (baseint_t *)(key + SUPERKEYSIZE + ((s * KEYSIZE))));

                field_add_reduce(&t, &t, &k);

                acc[s + 1] = acc[0];

                for (int j = 1; j < s + 1; j++) {
                    field_add_reduce(acc + s + 1, acc + s + 1, acc + j);
                }
                field_mul_reduce(acc + s + 1, acc + s + 1, &t);
                memset(acc, 0, sizeof(field_elem_t));
            } else {
                INNERPOLY(acc, in, inlen, key, 1);
                in += SUPERBLOCKSIZE;
                inlen -= SUPERBLOCKSIZE;
            }
        } else {
            memset(acc, 0, sizeof(field_elem_t));
        }

        for (uint64_t i = 1, b = n; b > 0; b >>= 1, i++) {
            if (b & 1ULL) {
                field_add_reduce(acc, acc, acc + i);
            }
        }
#if EXPLICIT_LENGTH_ENCODE
        LENGTH_ENCODING(acc, acc, key, keylen, msglen);
        reduce(acc, acc);
#endif
        pack_field_elem((baseint_t *)tag_packed, acc);
        free(acc);
    }

    transform_field_elem(out, OUTPUTSIZE, tag_packed, BUFFSIZE);
}
