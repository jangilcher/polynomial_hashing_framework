# MIT License
#
# Copyright (c) 2023 Jan Gilcher, Jérôme Govinden
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import pathlib
import ctypes
import random
import unittest
from functools import reduce
from math import ceil
from numpy.polynomial import Polynomial as P


def cumsum(arr):
    return reduce((lambda x, y: x + [x[-1] + y]), arr, [0])


def bitfield(n):
    return [int(digit) for digit in bin(n)[2:]]


def int_to_poly(n):
    return P(list(reversed(bitfield(n))))


def poly_to_int(p):
    return int("".join(map(lambda x: str(int(x)), reversed(p.coef % 2))), 2)


def coeffs_to_poly(coeffs):
    diffs = []
    for i, j in zip(coeffs, coeffs[1:]):
        diffs.append(i - j - 1)
    diffs.append(coeffs[-1] - 0)
    arr = []
    for i in reversed(diffs):
        arr += [0] * i + [1]
    return P(arr)


class TestArith(unittest.TestCase):
    def __init__(
        self,
        name,
        binname,
        iterations=100000,
        fieldsize=128,
        coeffs=[128, 7, 2, 1, 0],
        wordsize=64,
        limbsizes=[64, 64],
        method="schoolbook",
        primename=None,
        blocksize=16,
        keysize=16,
    ):
        super(TestArith, self).__init__(name)
        self.iterations = iterations
        self.fieldsize = fieldsize
        self.wordsize = wordsize
        self.limbsizes = limbsizes
        self.method = method
        self.primename = primename
        self.blocksize = blocksize
        self.keysize = keysize
        self.coeffs = coeffs
        self.poly = coeffs_to_poly(coeffs)
        self.libname = pathlib.Path().absolute() / "bin" / f"{binname}_arithmetic.so"
        # self.libname = (
        #     pathlib.Path().absolute()
        #     / "bin"
        #     / f'bf_arithmetic_{fieldsize}_{"_".join(map(str,limbsizes))}_{wordsize}_{method}.so'
        # )

    def setUp(self):
        self.lib = ctypes.CDLL(str(self.libname))

    # def int_to_dfield_elem(self, a):
    #     sizes = list(map(lambda x: 2*x, self.limbsizes))
    #     cumsizes = cumsum(sizes)
    #     if log2(a)>self.wordsize*2*len(self.limbsizes):
    #         raise Exception('Integer too large')
    #     bits = ceil(log2(a))
    #     arr = []
    #     for i, j in zip(sizes, self.limbsizes):
    #         arr.append(self.to_long_int(a % (2**i)))
    #         a = (a - a % (2**i)) // (2**j)
    #     return self.DField_Elem((self.long_int * len(arr))(*arr))

    # def rand_dfield_elem(self, full=True):
    #     sizes = list(map(lambda x: 2*x, self.limbsizes))
    #     cumsizes = cumsum(self.limbsizes)
    #     arr = []
    #     res = 0
    #     for i, j in zip(sizes, cumsizes):
    #         if full:
    #             r = random.randrange(0, 2**(self.wordsize-1))
    #         else:
    #             r = random.randrange(0, 2**i)
    #         arr.append(self.to_long_int(r))
    #         res += r*(2**j)
    #     return self.DField_Elem((self.long_int * len(arr))(*arr)), res

    # def dfield_elem_to_int(self, elem):
    #     return sum(map(lambda x: self.from_long_int(x[0])*(2**x[1]), zip(elem.val, cumsum(self.limbsizes))))

    # def field_elem_to_int(self, elem):
    #     return sum(map(lambda x: x[0]*(2**x[1]), zip(elem.val, cumsum(self.limbsizes))))

    def int_to_array(self, i):
        cumsizes = cumsum([self.wordsize] * ceil(self.fieldsize / self.wordsize))
        arr = []
        for j in cumsizes[:-1]:
            arr.append((i // 2**j) % (2**self.wordsize))
        if ceil(self.fieldsize / self.wordsize) % 2 == 1:
            arr.append(0)
        return (ctypes.c_uint64 * len(arr))(*arr)

    def int_to_darray(self, i):
        cumsizes = cumsum([self.wordsize] * ceil(2 * self.fieldsize / self.wordsize))
        arr = []
        for j in cumsizes[:-1]:
            arr.append((i // 2**j) % (2**self.wordsize))
        if ceil(self.fieldsize / self.wordsize) % 2 == 1:
            arr = (
                arr[0 : ceil(self.fieldsize / self.wordsize)]
                + [0]
                + arr[ceil(self.fieldsize / self.wordsize) :]
                + [0]
            )
        return (ctypes.c_uint64 * len(arr))(*arr)

    def array_to_int(self, arr):
        res = 0
        for i, a in enumerate(arr):
            res += a * 2 ** (self.wordsize * i)
        return res

    def test_mul(self):
        for t in range(0, self.iterations):
            with self.subTest(t=t):
                a = random.getrandbits(self.fieldsize)
                b = random.getrandbits(self.fieldsize)
                arr1 = self.int_to_array(a)
                arr2 = self.int_to_array(b)
                a_poly = int_to_poly(a)
                b_poly = int_to_poly(b)
                res_arr = [0] * len(arr1)
                res = (ctypes.c_uint64 * len(res_arr))(*res_arr)
                ref = (a_poly * b_poly) % self.poly
                self.lib.mul_test_test(
                    ctypes.pointer(res), ctypes.pointer(arr1), ctypes.pointer(arr2)
                )
                self.assertEqual(self.array_to_int(res), poly_to_int(ref))

    def test_add(self):
        for t in range(0, self.iterations):
            with self.subTest(t=t):
                a = random.getrandbits(self.fieldsize)
                b = random.getrandbits(self.fieldsize)
                arr1 = self.int_to_array(a)
                arr2 = self.int_to_array(b)
                res_arr = [0] * len(arr1)
                res = (ctypes.c_uint64 * len(res_arr))(*res_arr)
                ref_arr = []
                for a, b in zip(arr1, arr2):
                    ref_arr.append(a ^ b)
                ref = (ctypes.c_uint64 * len(ref_arr))(*ref_arr)
                self.lib.add_test_test(
                    ctypes.pointer(res), ctypes.pointer(arr1), ctypes.pointer(arr2)
                )
                for i, j in zip(res, ref):
                    self.assertEqual(i, j)

    def test_sqr(self):
        for t in range(0, self.iterations):
            with self.subTest(t=t):
                a = random.getrandbits(self.fieldsize)
                arr1 = self.int_to_array(a)
                a_poly = int_to_poly(a)
                res_arr = [0] * len(arr1)
                res = (ctypes.c_uint64 * len(res_arr))(*res_arr)
                ref = (a_poly * a_poly) % self.poly
                self.lib.sqr_test_test(ctypes.pointer(res), ctypes.pointer(arr1))
                self.assertEqual(self.array_to_int(res), poly_to_int(ref))

    def test_mul_no_carry(self):
        for t in range(0, self.iterations):
            with self.subTest(t=t):
                a = random.getrandbits(self.fieldsize)
                b = random.getrandbits(self.fieldsize)
                arr1 = self.int_to_array(a)
                arr2 = self.int_to_array(b)
                a_poly = int_to_poly(a)
                b_poly = int_to_poly(b)
                res_arr = [0] * ceil(self.fieldsize / 128) * 4
                res = (ctypes.c_uint64 * len(res_arr))(*res_arr)
                ref = a_poly * b_poly
                self.lib.mul_no_carry_test(
                    ctypes.pointer(res), ctypes.pointer(arr1), ctypes.pointer(arr2)
                )
                if ceil(self.fieldsize / self.wordsize) % 2 == 1:
                    self.assertEqual(
                        self.array_to_int(
                            res[0 : len(res) // 2 - 1] + res[len(res) // 2 : -1]
                        ),
                        poly_to_int(ref),
                    )
                else:
                    self.assertEqual(self.array_to_int(res), poly_to_int(ref))

    # def test_add_mix(self):
    #     for t in range(0, self.iterations):
    #         with self.subTest(t=t):
    #             b = random.randrange(0, self.p)
    #             arr1, _ = self.rand_dfield_elem(full=False)
    #             arr2 = self.int_to_field_elem(b)
    #             res = self.DField_Elem()
    #             ref_arr = []
    #             for a, b in zip(arr1.val, arr2.val):
    #                 ref_arr.append(self.to_long_int(self.from_long_int(a) + b))
    #             ref = (self.long_int * len(ref_arr))(*ref_arr)
    #             self.lib.field_add_mix(ctypes.pointer(
    #                 res), ctypes.pointer(arr1), ctypes.pointer(arr2))
    #             for i, j in zip(res.val, ref):
    #                 self.assertEqual(self.from_long_int(i),self.from_long_int(j))

    def test_add_dbl(self):
        for t in range(0, self.iterations):
            with self.subTest(t=t):
                a = random.getrandbits(2 * self.fieldsize)
                b = random.getrandbits(2 * self.fieldsize)
                arr1 = self.int_to_darray(a)
                arr2 = self.int_to_darray(b)
                res_arr = [0] * len(arr1)
                res = (ctypes.c_uint64 * len(res_arr))(*res_arr)
                ref_arr = []
                for a, b in zip(arr1, arr2):
                    ref_arr.append(a ^ b)
                ref = (ctypes.c_uint64 * len(ref_arr))(*ref_arr)
                self.lib.add_dbl_test(
                    ctypes.pointer(res), ctypes.pointer(arr1), ctypes.pointer(arr2)
                )
                for i, j in zip(res, ref):
                    self.assertEqual(i, j)

    def test_sqr_no_carry(self):
        for t in range(0, self.iterations):
            with self.subTest(t=t):
                a = random.getrandbits(self.fieldsize)
                arr1 = self.int_to_array(a)
                a_poly = int_to_poly(a)
                res_arr = [0] * ceil(self.fieldsize / 128) * 4
                res = (ctypes.c_uint64 * len(res_arr))(*res_arr)
                ref = a_poly * a_poly
                self.lib.sqr_no_carry_test(ctypes.pointer(res), ctypes.pointer(arr1))
                if ceil(self.fieldsize / self.wordsize) % 2 == 1:
                    self.assertEqual(
                        self.array_to_int(
                            res[0 : len(res) // 2 - 1] + res[len(res) // 2 : -1]
                        ),
                        poly_to_int(ref),
                    )
                else:
                    self.assertEqual(self.array_to_int(res), poly_to_int(ref))

    # def test_reduce(self):
    #     for t in range(0, self.iterations):
    #         with self.subTest(t=t):
    #             a = random.randrange(0, self.p)
    #             ref = a % self.p
    #             res = self.Field_Elem()
    #             a = self.int_to_field_elem(a)
    #             self.lib.reduce(ctypes.pointer(res), ctypes.pointer(a))
    #             self.assertEqual(self.field_elem_to_int(res), ref)

    #             a = random.randrange(self.p, 2**self.pi)
    #             ref = a % self.p
    #             res = self.Field_Elem()
    #             a = self.int_to_field_elem(a)
    #             self.lib.reduce(ctypes.pointer(res), ctypes.pointer(a))
    #             self.assertEqual(self.field_elem_to_int(res), ref)

    def test_carry_round(self):
        for t in range(0, self.iterations):
            with self.subTest(t=t):
                a = random.getrandbits(2 * self.fieldsize - 1)
                arr1 = self.int_to_darray(a)
                a_poly = int_to_poly(a)
                ref = a_poly % self.poly
                res_arr = [0] * ceil(self.fieldsize / 128) * 2
                res = (ctypes.c_uint64 * len(res_arr))(*res_arr)
                self.lib.carry_test(ctypes.pointer(res), ctypes.pointer(arr1))
                self.assertEqual(poly_to_int(ref), self.array_to_int(res))


if __name__ == "__main__":
    # unittest.main()
    suite = unittest.TestSuite()
    suite.addTest(TestArith("test_pack", iterations=1))
    suite.addTest(TestArith("test_unpack", iterations=1))
    suite.addTest(TestArith("test_mul", iterations=1))
    suite.addTest(TestArith("test_add", iterations=1))
    suite.addTest(TestArith("test_sqr", iterations=1))
    suite.addTest(TestArith("test_carry_round", iterations=1))
    suite.addTest(TestArith("test_sqr_no_carry", iterations=1))
    suite.addTest(TestArith("test_add_dbl", iterations=1))
    suite.addTest(TestArith("test_mul_no_carry", iterations=1))
    unittest.TextTestRunner(verbosity=2).run(suite)
