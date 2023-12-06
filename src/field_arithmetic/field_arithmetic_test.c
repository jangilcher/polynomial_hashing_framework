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

void field_mul_test(field_elem_t *res, field_elem_t *a, field_elem_t *b) {
    field_mul(res, a, b);
}

void field_add_test(field_elem_t *res, field_elem_t *a, field_elem_t *b) {
    field_add(res, a, b);
}

void field_sqr_test(field_elem_t *res, field_elem_t *a) { field_sqr(res, a); }

void field_mul_no_carry_test(dfield_elem_t *res, field_elem_t *a,
                             field_elem_t *b) {
    field_mul_no_carry(res, a, b);
}

void field_mul_reduce_test(field_elem_t *res, field_elem_t *a,
                           field_elem_t *b) {
    field_mul_reduce(res, a, b);
}

void field_add_mix_test(dfield_elem_t *res, dfield_elem_t *a, field_elem_t *b) {
    field_add_mix(res, a, b);
}

void field_add_dbl_test(dfield_elem_t *res, dfield_elem_t *a,
                        dfield_elem_t *b) {
    field_add_dbl(res, a, b);
}

void field_add_reduce_test(field_elem_t *res, field_elem_t *a,
                           field_elem_t *b) {
    field_add_reduce(res, a, b);
}

void field_sqr_no_carry_test(dfield_elem_t *res, field_elem_t *a) {
    field_sqr_no_carry(res, a);
}

void field_sqr_reduce_test(field_elem_t *res, field_elem_t *a) {
    field_sqr_reduce(res, a);
}

void carry_round_test(field_elem_t *res, dfield_elem_t *a) {
    carry_round(res, a);
}

void reduce_test(field_elem_t *res, field_elem_t *a) { reduce(res, a); }

void pack_field_elem_test(baseint_t *res, field_elem_t *a) {
    pack_field_elem(res, a);
}

void unpack_key_test(field_elem_t *res, baseint_t *a) { unpack_key(res, a); }

void unpack_field_elem_test(field_elem_t *res, baseint_t *a) {
    unpack_field_elem(res, a);
}

void unpack_and_encode_field_elem_test(field_elem_t *res, baseint_t *a) {
    unpack_and_encode_field_elem(res, a);
}

void unpack_and_encode_last_field_elem_test(field_elem_t *res, baseint_t *a,
                                            size_t size) {
    unpack_and_encode_last_field_elem(res, a, size);
}
