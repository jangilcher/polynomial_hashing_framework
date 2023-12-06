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

#include "field_arithmetic.h"
#include <stdint.h>

int field_mul(field_elem_t *res, field_elem_t *a, field_elem_t *b);

int field_add(field_elem_t *res, field_elem_t *a, field_elem_t *b);

int field_sqr(field_elem_t *res, field_elem_t *a);

int field_mul_no_carry(dfield_elem_t *res, field_elem_t *a, field_elem_t *b);

int field_mul_reduce(field_elem_t *res, field_elem_t *a, field_elem_t *b);

int field_add_mix(dfield_elem_t *res, dfield_elem_t *a, field_elem_t *b);

int field_add_dbl(dfield_elem_t *res, dfield_elem_t *a, dfield_elem_t *b);

int field_add_reduce(field_elem_t *res, field_elem_t *a, field_elem_t *b);

int field_sqr_no_carry(dfield_elem_t *res, field_elem_t *a);

int field_sqr_reduce(field_elem_t *res, field_elem_t *a);

int carry_round(field_elem_t *res, dfield_elem_t *a);

int reduce(field_elem_t *res, field_elem_t *a);

int pack_field_elem(baseint_t *res, field_elem_t *a);

int unpack_key(field_elem_t *res, baseint_t *a);

int unpack_field_elem(field_elem_t *res, baseint_t *a);

int unpack_and_encode_field_elem(field_elem_t *res, baseint_t *a);

int unpack_and_encode_last_field_elem(field_elem_t *res, baseint_t *a,
                                      size_t size);
