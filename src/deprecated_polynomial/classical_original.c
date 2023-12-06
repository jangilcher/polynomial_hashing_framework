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

#include "../field_arithmetic/field_arithmetic.h"
#include "../transform/transform.h"
#include <stddef.h>
#include <string.h>

#if defined(INNERPOLY_H) && defined(INNERPOLY)
#include INNERPOLY_H
#endif

void classical_original(unsigned char *out, const unsigned char *in,
                        unsigned long long inlen, const unsigned char *key) {
    field_elem_t acc = {0};
    field_elem_t a = {0};
    field_elem_t k = {0};
    unsigned char buff[BUFFSIZE] = {0};
    unsigned char tag_packed[BUFFSIZE] = {0};
#if KEYTRANSFORM == none_transform
    unpack_field_elem(&k, (baseint_t *)key);
#else
    unsigned char transkey[BUFFSIZE] = {0};
    transform_key(transkey, BUFFSIZE, key, KEYSIZE);
    unpack_field_elem(&k, (baseint_t *)transkey);
#endif
    while (inlen >= BLOCKSIZE) {

#ifdef LASTBLOCKENCODING
        unpack_field_elem(&a, (baseint_t *)in);
#elif MSGTRANSFORM == none_transform
        unpack_and_encode_field_elem(&a, (baseint_t *)in);
#else
        transform_msg(buff, BUFFSIZE, in, BLOCKSIZE);
        unpack_and_encode_field_elem(&a, (baseint_t *)buff);
#endif
        field_add(&acc, &acc, &a);
        field_mul(&acc, &acc, &k);
        inlen -= BLOCKSIZE;
        in += BLOCKSIZE;
    }
#ifdef LASTBLOCKENCODING
    if (inlen >= 0)
#else
    if (inlen)
#endif
    {
        memset(buff, 0, BUFFSIZE);
#if MSGTRANSFORM == none_transform
        memcpy(buff, in, inlen);
#else
        transform_msg(buff, BUFFSIZE, in, inlen);
#endif
        unpack_and_encode_last_field_elem(&a, (baseint_t *)buff, inlen);
        field_add(&acc, &acc, &a);
        field_mul(&acc, &acc, &k);
    }
    // pack_field_elem((baseint_t *) out, &acc);

    pack_field_elem((baseint_t *)tag_packed, &acc);
    transform_field_elem(out, OUTPUTSIZE, tag_packed, BUFFSIZE);
}
