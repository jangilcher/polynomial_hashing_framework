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

#ifndef __TRANSFORM_H
#define __TRANSFORM_H
#include <stddef.h>
#include <stdint.h>

#ifndef KEYINCLUDE_H
#define KEYINCLUDE_H "identity.h"
#endif
#ifndef MSGINCLUDE_H
#define MSGINCLUDE_H "identity.h"
#endif
#ifndef FIELDELEMINCLUDE_H
#define FIELDELEMINCLUDE_H "identity.h"
#endif
#include KEYINCLUDE_H
#include MSGINCLUDE_H
#include FIELDELEMINCLUDE_H

#ifndef KEYTRANSFORM
#define KEYTRANSFORM identity_transform
#endif
#ifndef MSGTRANSFORM
#define MSGTRANSFORM identity_transform
#endif
#ifndef FIELDELEMTRANSFORM
#define FIELDELEMTRANSFORM identity_transform
#endif

static void (*const transform_key)(uint8_t *out, size_t out_len,
                                   const uint8_t *in,
                                   size_t in_len) = KEYTRANSFORM;

static void (*const transform_msg)(uint8_t *out, size_t out_len,
                                   const uint8_t *in,
                                   size_t in_len) = MSGTRANSFORM;

static void (*const transform_field_elem)(uint8_t *out, size_t out_len,
                                          const uint8_t *in,
                                          size_t in_len) = FIELDELEMTRANSFORM;

inline void transform_msg2(uint8_t *out, size_t out_len, const uint8_t *in,
                           size_t in_len) {
    size_t min_len = in_len >= out_len ? out_len : in_len;
    memcpy(out, in, min_len);
}

#endif
