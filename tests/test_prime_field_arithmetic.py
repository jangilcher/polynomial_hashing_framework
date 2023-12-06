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
from math import ceil, log2


def cumsum(arr):
    return reduce((lambda x, y: x + [x[-1] + y]), arr, [0])


class TestArith(unittest.TestCase):
    def __init__(
        self,
        name,
        binname,
        iterations=100000,
        primetype="0",
        pi=130,
        delta=5,
        wordsize=64,
        limbsizes=[44, 44, 42],
        method="schoolbook",
        primename=None,
        blocksize=16,
        keysize=16,
    ):
        super(TestArith, self).__init__(name)
        self.binname = binname
        self.pi = pi
        self.delta = delta
        self.p = 2**pi - delta
        self.wordsize = wordsize
        self.limbsizes = limbsizes
        self.iterations = iterations
        self.blocksize = blocksize
        self.keysize = keysize
        if wordsize == 64:
            self.base_int = ctypes.c_uint64

            class c_uint128(ctypes.Structure):
                _pack_ = 16
                _fields_ = [("lower", ctypes.c_uint64), ("upper", ctypes.c_uint64)]

            self.long_int = c_uint128

            def to_long_int64(a):
                return c_uint128(a % (2**64), a // (2**64))

            def from_long_int64(a):
                return a.lower + 2**64 * a.upper

            class Field_Elem64(ctypes.Structure):
                _fields_ = [("val", ctypes.c_uint64 * len(limbsizes))]

            class DField_Elem64(ctypes.Structure):
                _fields_ = [("val", c_uint128 * len(limbsizes))]

            self.to_long_int = to_long_int64
            self.from_long_int = from_long_int64
            self.Field_Elem = Field_Elem64
            self.DField_Elem = DField_Elem64
        else:
            self.base_int = ctypes.c_uint32
            self.long_int = ctypes.c_uint64

            def to_long_int32(a):
                return a

            def from_long_int32(a):
                return a

            class Field_Elem32(ctypes.Structure):
                _fields_ = [("val", ctypes.c_uint32 * len(limbsizes))]

            class DField_Elem32(ctypes.Structure):
                _fields_ = [("val", ctypes.c_uint64 * len(limbsizes))]

            self.to_long_int = to_long_int32
            self.from_long_int = from_long_int32
            self.Field_Elem = Field_Elem32
            self.DField_Elem = DField_Elem32

        self.libname = pathlib.Path().absolute() / "bin" / f"{binname}_arithmetic.so"
        # if primetype == "0":
        #     self.libname /= f"pf_arithmetic_0_{primename}_{'_'.join(map(str,limbsizes))}_{wordsize}_{method}.so"
        # elif primetype == "1":
        #     self.libname /= f"pf_arithmetic_1_{primename}_{'_'.join(map(str,limbsizes))}_{wordsize}_{method}.so"

    def setUp(self):
        self.lib = ctypes.CDLL(str(self.libname))

    def int_to_field_elem(self, a):
        cumsizes = cumsum(self.limbsizes)
        arr = []
        for i, j in zip(self.limbsizes, cumsizes):
            arr.append((a // (2**j)) % (2**i))
        return self.Field_Elem((self.base_int * len(arr))(*arr))

    def int_to_dfield_elem(self, a):
        sizes = list(map(lambda x: 2 * x, self.limbsizes))
        cumsizes = cumsum(sizes)
        if log2(a) > self.wordsize * 2 * len(self.limbsizes):
            raise Exception("Integer too large")
        bits = ceil(log2(a))
        arr = []
        for i, j in zip(sizes, self.limbsizes):
            arr.append(self.to_long_int(a % (2**i)))
            a = (a - a % (2**i)) // (2**j)
        return self.DField_Elem((self.long_int * len(arr))(*arr))

    def rand_dfield_elem(self, full=True):
        sizes = list(map(lambda x: 2 * x, self.limbsizes))
        cumsizes = cumsum(self.limbsizes)
        arr = []
        res = 0
        for i, j in zip(sizes, cumsizes):
            if full:
                r = random.randrange(0, 2 ** (self.wordsize - 1))
            else:
                r = random.randrange(0, 2**i)
            arr.append(self.to_long_int(r))
            res += r * (2**j)
        return self.DField_Elem((self.long_int * len(arr))(*arr)), res

    def dfield_elem_to_int(self, elem):
        return sum(
            map(
                lambda x: self.from_long_int(x[0]) * (2 ** x[1]),
                zip(elem.val, cumsum(self.limbsizes)),
            )
        )

    def field_elem_to_int(self, elem):
        return sum(
            map(lambda x: x[0] * (2 ** x[1]), zip(elem.val, cumsum(self.limbsizes)))
        )

    def int_to_array(self, i):
        cumsizes = cumsum([self.wordsize] * ceil(self.pi / self.wordsize))
        arr = []
        for j in cumsizes[:-1]:
            arr.append((i // 2**j) % (2**self.wordsize))
        return (self.base_int * len(arr))(*arr)

    def array_to_int(self, arr):
        res = 0
        for i, a in enumerate(arr):
            res += a * 2 ** (self.wordsize * i)
        return res

    def test_mul(self):
        for t in range(0, self.iterations):
            with self.subTest(t=t):
                a = random.randrange(0, self.p)
                b = random.randrange(0, self.p)
                arr1 = self.int_to_field_elem(a)
                arr2 = self.int_to_field_elem(b)
                res = self.Field_Elem()
                self.lib.field_mul_test(
                    ctypes.pointer(res), ctypes.pointer(arr1), ctypes.pointer(arr2)
                )
                for i, j in zip(res.val, self.int_to_field_elem((a * b) % self.p).val):
                    self.assertEqual(i, j)

    def test_add(self):
        for t in range(0, self.iterations):
            with self.subTest(t=t):
                a = random.randrange(0, self.p)
                b = random.randrange(0, self.p)
                arr1 = self.int_to_field_elem(a)
                arr2 = self.int_to_field_elem(b)
                res = self.Field_Elem()
                ref_arr = []
                for a, b in zip(arr1.val, arr2.val):
                    ref_arr.append(a + b)
                ref = (self.base_int * len(ref_arr))(*ref_arr)
                self.lib.field_add_test(
                    ctypes.pointer(res), ctypes.pointer(arr1), ctypes.pointer(arr2)
                )
                for i, j in zip(res.val, ref):
                    self.assertEqual(i, j)

    def test_sqr(self):
        for t in range(0, self.iterations):
            with self.subTest(t=t):
                a = random.randrange(0, self.p)
                arr1 = self.int_to_field_elem(a)
                res = self.Field_Elem()
                self.lib.field_sqr_test(ctypes.pointer(res), ctypes.pointer(arr1))
                for i, j in zip(res.val, self.int_to_field_elem((a**2) % self.p).val):
                    self.assertEqual(i, j)

    def test_mul_no_carry(self):
        for t in range(0, self.iterations):
            with self.subTest(t=t):
                a = random.randrange(0, 2**self.pi)
                b = random.randrange(0, 2**self.pi)
                arr1 = self.int_to_field_elem(a)
                arr2 = self.int_to_field_elem(b)
                res = self.DField_Elem()

                self.lib.field_mul_no_carry_test(
                    ctypes.pointer(res), ctypes.pointer(arr1), ctypes.pointer(arr2)
                )
                self.assertEqual(self.dfield_elem_to_int(res) % self.p, a * b % self.p)

    def test_add_mix(self):
        for t in range(0, self.iterations):
            with self.subTest(t=t):
                b = random.randrange(0, self.p)
                arr1, _ = self.rand_dfield_elem(full=False)
                arr2 = self.int_to_field_elem(b)
                res = self.DField_Elem()
                ref_arr = []
                for a, b in zip(arr1.val, arr2.val):
                    ref_arr.append(self.to_long_int(self.from_long_int(a) + b))
                ref = (self.long_int * len(ref_arr))(*ref_arr)
                self.lib.field_add_mix_test(
                    ctypes.pointer(res), ctypes.pointer(arr1), ctypes.pointer(arr2)
                )
                for i, j in zip(res.val, ref):
                    self.assertEqual(self.from_long_int(i), self.from_long_int(j))

    def test_add_dbl(self):
        for t in range(0, self.iterations):
            with self.subTest(t=t):
                arr1, _ = self.rand_dfield_elem(full=False)
                arr2, _ = self.rand_dfield_elem(full=False)
                res = self.DField_Elem()
                ref_arr = []
                for a, b in zip(arr1.val, arr2.val):
                    ref_arr.append(
                        self.to_long_int(self.from_long_int(a) + self.from_long_int(b))
                    )
                ref = (self.long_int * len(ref_arr))(*ref_arr)
                self.lib.field_add_dbl_test(
                    ctypes.pointer(res), ctypes.pointer(arr1), ctypes.pointer(arr2)
                )
                for i, j in zip(res.val, ref):
                    self.assertEqual(self.from_long_int(i), self.from_long_int(j))

    def test_sqr_no_carry(self):
        for t in range(0, self.iterations):
            with self.subTest(t=t):
                a = random.randrange(0, 2**self.pi)
                arr1 = self.int_to_field_elem(a)
                res = self.DField_Elem()

                self.lib.field_sqr_no_carry_test(
                    ctypes.pointer(res), ctypes.pointer(arr1)
                )
                self.assertEqual(self.dfield_elem_to_int(res) % self.p, a * a % self.p)

    def test_reduce(self):
        for t in range(0, self.iterations):
            with self.subTest(t=t):
                a = random.randrange(0, self.p)
                ref = a % self.p
                res = self.Field_Elem()
                a = self.int_to_field_elem(a)
                self.lib.reduce_test(ctypes.pointer(res), ctypes.pointer(a))
                self.assertEqual(self.field_elem_to_int(res), ref)

                a = random.randrange(self.p, 2**self.pi)
                ref = a % self.p
                res = self.Field_Elem()
                a = self.int_to_field_elem(a)
                self.lib.reduce_test(ctypes.pointer(res), ctypes.pointer(a))
                self.assertEqual(self.field_elem_to_int(res), ref)

    def test_carry_round(self):
        for t in range(0, self.iterations):
            with self.subTest(t=t):
                a = random.randrange(0, self.p)
                self.assertEqual(self.field_elem_to_int(self.int_to_field_elem(a)), a)
                res = self.Field_Elem()
                ref = (a // (2**self.pi)) * self.delta + (a % (2**self.pi))
                a = self.int_to_dfield_elem(a)
                self.lib.carry_round_test(ctypes.pointer(res), ctypes.pointer(a))
                res = self.field_elem_to_int(res)
                self.assertEqual(res, ref)

                res = self.Field_Elem()
                a, b = self.rand_dfield_elem()
                self.lib.carry_round_test(ctypes.pointer(res), ctypes.pointer(a))
                ref = (b // (2**self.pi)) * self.delta + (b % (2**self.pi))
                res = self.field_elem_to_int(res)
                self.assertEqual(res, ref)

    def test_unpack_msg(self):
        for t in range(0, self.iterations):
            with self.subTest(t=t):
                a = random.randrange(0, min(2 ** (self.blocksize * 8), self.p))
                res = self.Field_Elem()
                ref = self.int_to_field_elem(a)
                arr = self.int_to_array(a)
                self.lib.unpack_field_elem_test(ctypes.pointer(res), arr)
                for i, j in zip(res.val, ref.val):
                    self.assertEqual(i, j)

    def test_unpack_key(self):
        for t in range(0, self.iterations):
            with self.subTest(t=t):
                a = random.randrange(0, min(2 ** (self.keysize * 8), self.p))
                res = self.Field_Elem()
                ref = self.int_to_field_elem(a)
                arr = self.int_to_array(a)
                self.lib.unpack_key_test(ctypes.pointer(res), arr)
                for i, j in zip(res.val, ref.val):
                    self.assertEqual(i, j)

    def test_pack(self):
        for t in range(0, self.iterations):
            with self.subTest(t=t):
                a = random.randrange(0, self.p)
                size = ceil(self.pi / self.wordsize)
                res_arr = [0] * size
                res = (self.base_int * size)(*res_arr)
                elem = self.int_to_field_elem(a)
                ref = self.int_to_array(a)
                self.lib.pack_field_elem_test(res, ctypes.pointer(elem))
                for i, j in zip(res, ref):
                    self.assertEqual(i, j)

    def test_mul_precomputed(self):
        for t in range(0, self.iterations):
            with self.subTest(t=t):
                a = random.randrange(0, self.p)
                b = random.randrange(0, self.p)
                arr1 = self.int_to_field_elem(a)
                arr2 = self.int_to_field_elem(b)
                res = self.Field_Elem()
                self.lib.field_mul_precomputed_test(
                    ctypes.pointer(res), ctypes.pointer(arr1), ctypes.pointer(arr2)
                )
                for i, j in zip(res.val, self.int_to_field_elem((a * b) % self.p).val):
                    self.assertEqual(i, j)

    def test_mul_precomputed_no_carry(self):
        for t in range(0, self.iterations):
            with self.subTest(t=t):
                a = random.randrange(0, 2**self.pi)
                b = random.randrange(0, 2**self.pi)
                arr1 = self.int_to_field_elem(a)
                arr2 = self.int_to_field_elem(b)
                res = self.DField_Elem()

                self.lib.field_mul_precomputed_no_carry_test(
                    ctypes.pointer(res), ctypes.pointer(arr1), ctypes.pointer(arr2)
                )
                self.assertEqual(self.dfield_elem_to_int(res) % self.p, a * b % self.p)

    def test_sqr_precomputed(self):
        for t in range(0, self.iterations):
            with self.subTest(t=t):
                a = random.randrange(0, self.p)
                arr1 = self.int_to_field_elem(a)
                res = self.Field_Elem()
                self.lib.field_sqr_precomputed_test(
                    ctypes.pointer(res), ctypes.pointer(arr1)
                )
                for i, j in zip(res.val, self.int_to_field_elem((a**2) % self.p).val):
                    self.assertEqual(i, j)

    def test_sqr_precomputed_no_carry(self):
        for t in range(0, self.iterations):
            with self.subTest(t=t):
                a = random.randrange(0, 2**self.pi)
                arr1 = self.int_to_field_elem(a)
                res = self.DField_Elem()

                self.lib.field_sqr_precomputed_no_carry_test(
                    ctypes.pointer(res), ctypes.pointer(arr1)
                )
                self.assertEqual(self.dfield_elem_to_int(res) % self.p, a * a % self.p)


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
