# MIT License
#
# Copyright (c) 2023 Jan Gilcher, Jérôme Govinden
#               2025 Jan Gilcher, Jérôme Govinden
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

import sys
from math import ceil
from typing import List, Optional
import itertools
from typing_extensions import override
from cpuinfo import get_cpu_info
from src.field_arithmetic.ArithmeticGenerator import ArithmeticGenerator


class FieldElem:
    def __init__(self, limbsize, numlimbs):
        self.limbsize = limbsize
        self.numlimbs = numlimbs


class BinaryFieldArithmeticGenerator(ArithmeticGenerator):
    def __init__(
        self,
        polynomial: List[int],
        cmulReduction=False,
        *args,
        **kwargs,
    ) -> None:
        super().__init__(*args, **kwargs)
        self.fieldsize: int = polynomial[0]
        cpu_info = get_cpu_info()
        if cpu_info["arch"] not in ["X86_32", "X86_64"]:
            raise NotImplementedError()
        if "pclmulqdq" not in cpu_info["flags"]:
            print(cpu_info["flags"])
            raise ValueError("Unsupported Platform")
        self.cmul: str = "pclmulqdq"
        self.limbbits = [64]
        self.numlimbs = ceil(self.fieldsize / self.limbbits[0])
        self.vecsize: int = 128
        self.numvecs: int = ceil(self.fieldsize / self.vecsize)
        self.numdlimbs: int = 2 * self.numlimbs
        self.polynomial: List[int] = polynomial
        self.cmulReduction = cmulReduction

    def _ADD(self, res, out, inp) -> None:
        print(f'{" "*self.tabdepth}{res} = _mm_xor_si128({out},{inp});', file=self.file)

    def _MUL(self, out, x, y, flag) -> None:
        print(
            f'{" "*self.tabdepth}{out} = _mm_clmulepi64_si128({x}, {y}, {flag});',
            file=self.file,
        )

    def _ANDVEC(self, res, out, inp) -> None:
        print(f'{" "*self.tabdepth}{res} = _mm_and_si128({out},{inp});', file=self.file)

    def _INC(self, out, inp) -> None:
        self._ADD(out, out, inp)

    def _RBITSHIFT64(self, val, shift) -> str:
        return f"_mm_srli_epi64({val}, {shift})"

    def _LBITSHIFT64(self, val, shift) -> str:
        return f"_mm_slli_epi64({val}, {shift})"

    def _RBYTESHIFT(self, val, shift) -> str:
        return f"_mm_bsrli_si128({val}, {shift})"

    def _LBYTESHIFT(self, val, shift) -> str:
        return f"_mm_bslli_si128({val}, {shift})"

    def _RSHIFT(self, val, shift) -> str:
        byteshift: int = shift // 8
        bitshift: int = shift % 8
        res: str = f"{val}"
        if byteshift != 0:
            res = f"_mm_bsrli_si128({res}, {byteshift})"
        if bitshift != 0:
            res = f"_mm_srli_epi64({res}, {bitshift})"
        return res

    def _LSHIFT(self, val, shift) -> str:
        byteshift: int = shift // 8
        bitshift: int = shift % 8
        res: str = f"{val}"
        if byteshift != 0:
            res = f"_mm_bslli_si128({res}, {byteshift})"
        if bitshift != 0:
            res = f"_mm_slli_epi64({res}, {bitshift})"
        return res

    def _LO(self, val) -> str:
        return f"_mm_unpacklo_epi64({val}, _mm_setzero_si128())"

    def _SWAP64(self, res, val) -> None:
        return print(
            f'{" "*self.tabdepth}{res} = _mm_shuffle_epi32({val}, 0x4E);',
            file=self.file,
        )

    def _UNPACKHI64(self, res, a, b) -> None:
        return print(
            f'{" "*self.tabdepth}{res} = _mm_unpackhi_epi64({a}, {b});', file=self.file
        )

    def _UNPACKLO64(self, res, a, b) -> None:
        return print(
            f'{" "*self.tabdepth}{res} = _mm_unpacklo_epi64({a}, {b});', file=self.file
        )

    @override
    def field_elem_get_one(self) -> None:
        self._function_header(
            f"{self.field_elem_t}",
            "field_elem_get_one",
            [],
        )
        self._startBody()
        retVal = (
            f"({self.field_elem_t})"
            + "{{{1, 0}"
            + ", {0, 0}" * (self.numvecs - 1)
            + "}}"
        )
        self._endBody(retVal=retVal)

    @override
    def pack_field_elem(self) -> None:
        self._function_header(
            "int",
            "pack_field_elem",
            [(f"{self.int_t}*", "res"), (f"const {self.field_elem_t}*", "a")],
        )
        self._startBody()
        self._CALL("memcpy", ["res", "a", f"{ceil(self.fieldsize / 8)}"])
        self._endBody()

    def limbmask(self, bits) -> str:
        return f"(((({self.int_t})1) << {bits}) - 1)"

    @override
    def unpack_field_elem(self) -> None:
        self._function_header(
            "int",
            "unpack_field_elem",
            [(f"{self.field_elem_t}*", "res"), (f"const {self.int_t}*", "a")],
        )
        self._startBody()
        self._CALL("memcpy", ["res", "a", f"{self.blocksize}"])
        self._endBody()

    @override
    def unpack_key(self) -> None:
        self._function_header(
            "int",
            "unpack_key",
            [(f"{self.field_elem_t}*", "res"), (f"const {self.int_t}*", "a")],
        )
        self._startBody()
        self._CALL("memcpy", ["res", "a", f"{self.keysize}"])
        self._endBody()

    @override
    def unpack_and_encode_key(self) -> None:
        if self.explicitKeyTransform:
            super().unpack_and_encode_key()
            return
        if self.keyClamp != 2 ** (self.keysize * 8) - 1:
            clamps: list[int] = [
                (self.keyClamp >> (self.wordsize * i)) % 2**self.wordsize
                for i in range(self.numlimbs)
            ]
            if self.numlimbs % 2:
                clamps.append(0)
            val = "{{"
            for i in range(0, len(clamps), 2):
                val += "{" + f"{hex(clamps[i])}ULL, {hex(clamps[i+1])}ULL" + "}, "
            val += "}}"
            self._declare_var(f"{self.field_elem_t}", "mask", val)
            # mask_bytes = self.keyClamp.to_bytes(ceil(self.keysize), byteorder="little")
            # mask_hex_chars = ",".join(map(hex, mask_bytes))
            # self._declare_var("char", f"mask[{self.keysize}]", "{" + f"{mask_hex_chars}" + "}")
        self._function_header(
            "int",
            "unpack_and_encode_key",
            [(f"{self.field_elem_t}*", "res"), (f"const {self.int_t}*", "a")],
        )
        self._startBody()
        self._CALL("memcpy", ["res", "a", f"{self.keysize}"])
        if self.keyClamp != 2 ** (self.keysize * 8) - 1:
            for i in range(self.numvecs):
                self._ANDVEC(f"res->val[{i}]", f"res->val[{i}]", f"mask.val[{i}]")
        self._endBody()

    @override
    def unpack_and_encode_field_elem(self) -> None:
        if self.explicitEncoding:
            super().unpack_and_encode_field_elem()
            return
        self._function_header(
            "int",
            "unpack_and_encode_field_elem",
            [(f"{self.field_elem_t}*", "res"), (f"const {self.int_t}*", "a")],
        )
        self._startBody()
        self._CALL("memcpy", ["res", "a", f"{self.blocksize}"])
        for i in range(self.numvecs):
            self._ASSIGN(
                f"res->val[{i}]",
                f"_mm_and_si128(_mm_set_epi64x({hex(self.encodingMask[2*i+1])}, \
                            {hex(self.encodingMask[2*i])}), res->val[{i}])",
            )
        self._endBody()

    @override
    def unpack_and_encode_last_field_elem(self) -> None:
        if self.explicitEncoding:
            super().unpack_and_encode_last_field_elem()
            return
        self._function_header(
            "int",
            "unpack_and_encode_last_field_elem",
            [
                (f"{self.field_elem_t}*", "res"),
                (f"const {self.int_t}*", "a"),
                ("size_t", "size"),
            ],
        )
        self._startBody()
        self._CALL("memcpy", ["res", "a", "size"])
        self._ASSIGN("((uint8_t*) (res->val))[size]", f"{self.encodingMSB}")
        for i in range(self.numvecs):
            self._ASSIGN(
                f"res->val[{i}]",
                f"_mm_and_si128(_mm_set_epi64x({hex(self.encodingMask[2*i+1])}, {hex(self.encodingMask[2*i])}), res->val[{i}])",
            )
        self._endBody()

    @override
    def field_addition(self) -> None:
        self._function_header(
            "int",
            "field_add",
            [
                (f"{self.field_elem_t}*", "res"),
                (f"const {self.field_elem_t}*", "a"),
                (f"const {self.field_elem_t}*", "b"),
            ],
        )
        self._startBody()
        for i in range(0, self.numvecs):
            self._ADD(f"res->val[{i}]", f"a->val[{i}]", f"b->val[{i}]")
        self._endBody()

    @override
    def field_addition_double(self) -> None:
        self._function_header(
            "int",
            "field_add_dbl",
            [
                (f"{self.dfield_elem_t}*", "res"),
                (f"const {self.dfield_elem_t}*", "a"),
                (f"const {self.dfield_elem_t}*", "b"),
            ],
        )
        self._startBody()
        for i in range(0, 2 * self.numvecs):
            self._ADD(f"res->val[{i}]", f"a->val[{i}]", f"b->val[{i}]")
        self._endBody()

    def _field_mul(self, res, inA, inB, acc, tmp):
        for i, j in itertools.product(range(self.numlimbs), repeat=2):
            self._coment(f"clmul(a[{i}], b[{j}])")
            self._MUL(
                f"{acc}",
                f"{inA}[{i//2}]",
                f"{inB}[{j//2}]",
                f"{(i % 2) + ((j % 2)<<4)}",
            )
            self._INC(f"{tmp}[{i+j}]", f"{acc}")
        print(file=self.file)
        if self.numlimbs % 2 == 0:
            for i in range(self.numlimbs):
                self._coment(
                    f"{res}[{i}] = {tmp}[{2*i}] + ({tmp}[{2*i+1}] <B< 8) + ({tmp}[{2*i-1}] >B> 8)"
                )
                self._ASSIGN(f"{res}[{i}]", f"{tmp}[{2*i}]")
                if (2 * i + 1) < 2 * self.numlimbs - 1:
                    self._ADD(
                        f"{res}[{i}]",
                        f"{res}[{i}]",
                        f"_mm_bslli_si128({tmp}[{2*i+1}], 8)",
                    )
                if (2 * i - 1) > 0:
                    self._ADD(
                        f"{res}[{i}]",
                        f"{res}[{i}]",
                        f"_mm_bsrli_si128({tmp}[{2*i-1}], 8)",
                    )
        else:
            for i in range(self.numvecs):
                self._coment(
                    f"{res}[{i}] = {tmp}[{2*i}] + ({tmp}[{2*i+1}] <B< 8) + ({tmp}[{2*i-1}] >B> 8)"
                )
                self._ASSIGN(f"{res}[{i}]", f"{tmp}[{2*i}]")
                if (2 * i + 1) < 2 * self.numvecs - 1:
                    self._ADD(
                        f"{res}[{i}]",
                        f"{res}[{i}]",
                        f"_mm_bslli_si128({tmp}[{2*i+1}], 8)",
                    )
                if (2 * i - 1) > 0:
                    self._ADD(
                        f"{res}[{i}]",
                        f"{res}[{i}]",
                        f"_mm_bsrli_si128({tmp}[{2*i-1}], 8)",
                    )
            self._ASSIGN(
                f"{res}[{self.numvecs-1}]",
                "_mm_and_si128(_mm_set_epi64x(0, 0xFFFFFFFFFFFFFFFF),"
                + f" {res}[{self.numvecs-1}])",
            )
            for j in range(self.numvecs, 2 * self.numvecs - 1):
                i: int = 2 * j - 1
                self._coment(
                    f"{res}[{j}] = {tmp}[{i}] + ({tmp}[{i+1}] <B< 8) + ({tmp}[{i-1}] >B> 8)"
                )
                self._ASSIGN(f"{res}[{j}]", f"{tmp}[{i}]")
                if (i + 1) < 2 * self.numlimbs - 1:
                    self._ADD(
                        f"{res}[{j}]",
                        f"{res}[{j}]",
                        f"_mm_bslli_si128({tmp}[{i+1}], 8)",
                    )
                if (i - 1) > 0:
                    self._ADD(
                        f"{res}[{j}]",
                        f"{res}[{j}]",
                        f"_mm_bsrli_si128({tmp}[{i-1}], 8)",
                    )
            self._ASSIGN(
                f"{res}[{2*self.numvecs-1}]",
                f"_mm_bsrli_si128({tmp}[{4*self.numvecs-4}], 8)",
            )

    @override
    def field_mul(
        self, doublecarryover: bool = False, doublecarry: bool = False
    ) -> None:
        self._function_header(
            "int",
            "field_mul",
            [
                (f"{self.field_elem_t}*", "res"),
                (f"const {self.field_elem_t}*", "a"),
                (f"const {self.field_elem_t}*", "b"),
            ],
        )
        self._startBody()
        self._declare_var(self.dfield_elem_t, "aa")
        self._declare_var("__m128i", "acc", "{0}")
        self._declare_var("__m128i", f"d[{max(2*(self.numlimbs)-1,1)}]", "{0}")

        self._field_mul(res="aa.val", inA="a->val", inB="b->val", acc="acc", tmp="d")
        if self.cmulReduction and self.fieldsize in [64, 128, 102, 256]:
            self._declare_var("__m128i", f"tt", "{0}")
            self._cmul_carry_round(res="res->val", inA="aa.val", mm128i_tmp="tt")
        else:
            self._declare_var("__m128i", f"tt[{self.numvecs}]", "{0}")
            self._declare_var(self.dfield_elem_t, "t")
            self._shift_carry_round(
                res="res->val", inA="aa.val", field_tmp="tt", dfield_tmp="t.val"
            )
        self._endBody()

    @override
    def field_mul_no_carry(self) -> None:
        self._function_header(
            "int",
            "field_mul_no_carry",
            [
                (f"{self.dfield_elem_t}*", "res"),
                (f"const {self.field_elem_t}*", "a"),
                (f"const {self.field_elem_t}*", "b"),
            ],
        )
        self._startBody()
        self._declare_var("__m128i", "acc", "{0}")
        self._declare_var("__m128i", f"d[{max(2*(self.numlimbs)-1,1)}]", "{0}")
        self._field_mul(res="res->val", inA="a->val", inB="b->val", acc="acc", tmp="d")
        self._endBody()

    def _shift_carry_round(self, res, inA, field_tmp, dfield_tmp) -> None:
        for i in range(self.numvecs):
            self._ADD(f"{res}[{i}]", f"{inA}[{i}]", f"{inA}[{i+self.numvecs}]")
        if self.numlimbs % 2 == 0:
            self._SWAP64(f"{dfield_tmp}[{0}]", f"{inA}[{self.numvecs}]")
            if self.numlimbs == 4:
                self._UNPACKLO64(
                    f"{dfield_tmp}[{1}]",
                    f"{dfield_tmp}[{0}]",
                    f"{inA}[{self.numlimbs//2 + 1}]",
                )
                self._UNPACKHI64(
                    f"{dfield_tmp}[{0}]",
                    f"{inA}[{self.numlimbs//2 + 1}]",
                    f"{dfield_tmp}[{0}]",
                )

            for k in range(self.numvecs):
                for j in self.polynomial[1:-1]:
                    self._coment(f"* x^{j}")
                    self._ADD(
                        f"{field_tmp}[{k}]",
                        f"{field_tmp}[{k}]",
                        self._RBITSHIFT64(f"{dfield_tmp}[{k}]", 64 - j),
                    )
                self._ADD(f"{res}[{k}]", f"{res}[{k}]", f"{field_tmp}[{k}]")

            self._ADD(
                f"{inA}[{self.numvecs}]",
                f"{inA}[{self.numvecs}]",
                f"_mm_and_si128(_mm_set_epi64x(0, 0xFFFFFFFFFFFFFFFF), {field_tmp}[{0}])",
            )
            for j in self.polynomial[1:-1]:
                self._coment(f"* x^{j}")
                for i in range(0, self.numvecs):
                    self._ADD(
                        f"{res}[{i}]",
                        f"{res}[{i}]",
                        self._LBITSHIFT64(f"{inA}[{self.numvecs + i}]", j),
                    )
        else:
            if self.numlimbs == 1:
                self._ASSIGN(f"{dfield_tmp}[{0}]", f"{inA}[{self.numvecs}]")
            else:
                self._UNPACKHI64(
                    f"{dfield_tmp}[{1}]",
                    f"{inA}[{self.numvecs}]",
                    f"{inA}[{self.numvecs + 1}]",
                )
                self._UNPACKLO64(
                    f"{dfield_tmp}[{0}]",
                    f"{inA}[{self.numvecs + 1}]",
                    f"{inA}[{self.numvecs}]",
                )
            for k in range(self.numvecs):
                for j in self.polynomial[1:-1]:
                    self._coment(f"* x^{j}")
                    self._ADD(
                        f"{field_tmp}[{k}]",
                        f"{field_tmp}[{k}]",
                        self._RBITSHIFT64(f"{dfield_tmp}[{k}]", 64 - j),
                    )
                self._ADD(f"{res}[{k}]", f"{res}[{k}]", f"{field_tmp}[{k}]")

            self._ADD(
                f"{inA}[{self.numvecs}]",
                f"{inA}[{self.numvecs}]",
                f"_mm_and_si128(_mm_set_epi64x(0, 0xFFFFFFFFFFFFFFFF), {field_tmp}[{0}])",
            )
            for j in self.polynomial[1:-1]:
                self._coment(f"* x^{j}")
                for i in range(0, self.numvecs):
                    self._ADD(
                        f"{res}[{i}]",
                        f"{res}[{i}]",
                        self._LBITSHIFT64(f"{inA}[{self.numvecs + i}]", j),
                    )

    def _cmul_carry_round(self, res, inA, mm128i_tmp) -> None:
        self._declare_var("__m128i", "poly")
        poly = 0
        for c in self.polynomial[1:]:
            poly += 1 << c
        self._ASSIGN("poly", f"_mm_set_epi64x(0, {poly})")
        if self.fieldsize == 64:
            self._MUL(f"{mm128i_tmp}", f"{inA}[1]", "poly", "0x0")
            self._ADD(f"{res}[0]", f"{inA}[0]", f"{mm128i_tmp}")
            self._ASSIGN(
                f"{res}[0]",
                f"_mm_and_si128(_mm_set_epi64x(0, 0xFFFFFFFFFFFFFFFF), {res}[0])",
            )
            self._MUL(f"{mm128i_tmp}", f"{mm128i_tmp}", "poly", "0x1")
            self._ADD(f"{res}[0]", f"{res}[0]", f"{mm128i_tmp}")
        elif self.fieldsize == 128:
            self._ASSIGN(f"{res}[0]", f"{inA}[0]")
            self._MUL(f"{mm128i_tmp}", f"{inA}[1]", "poly", "0x1")
            self._ADD(f"{res}[0]", f"{res}[0]", self._LBYTESHIFT(f"{mm128i_tmp}", "8"))
            self._ADD(
                f"{mm128i_tmp}", f"{inA}[1]", self._RBYTESHIFT(f"{mm128i_tmp}", "8")
            )
            self._MUL(f"{mm128i_tmp}", f"{mm128i_tmp}", "poly", "0x0")
            self._ADD(f"{res}[0]", f"{res}[0]", f"{mm128i_tmp}")
        elif self.fieldsize == 192:
            self._MUL(f"{mm128i_tmp}", f"{inA}[2]", "poly", "0x1")
            self._ADD(f"{res}[0]", f"{inA}[0]", self._LBYTESHIFT(f"{mm128i_tmp}", "8"))
            self._ADD(f"{res}[1]", f"{inA}[1]", self._RBYTESHIFT(f"{mm128i_tmp}", "8"))
            self._MUL(f"{mm128i_tmp}", f"{inA}[3]", "poly", "0x0")
            self._ADD(f"{res}[1]", f"{res}[1]", f"{mm128i_tmp}")
            self._ASSIGN(
                f"{res}[1]",
                f"_mm_and_si128(_mm_set_epi64x(0, 0xFFFFFFFFFFFFFFFF), {res}[1])",
            )
            self._ADD(
                f"{mm128i_tmp}", f"{inA}[2]", self._RBYTESHIFT(f"{mm128i_tmp}", "8")
            )
            self._MUL(f"{mm128i_tmp}", f"{mm128i_tmp}", "poly", "0x0")
            self._ADD(f"{res}[0]", f"{res}[0]", f"{mm128i_tmp}")
        elif self.fieldsize == 256:
            self._ASSIGN(f"{res}[0]", f"{inA}[0]")
            self._ASSIGN(f"{res}[1]", f"{inA}[1]")
            self._MUL(f"{mm128i_tmp}", f"{inA}[3]", "poly", "0x0")
            self._ADD(f"{res}[1]", f"{res}[1]", f"{mm128i_tmp}")
            self._MUL(f"{mm128i_tmp}", f"{inA}[2]", "poly", "0x1")
            self._ADD(f"{res}[0]", f"{res}[0]", self._LBYTESHIFT(f"{mm128i_tmp}", "8"))
            self._ADD(f"{res}[1]", f"{res}[1]", self._RBYTESHIFT(f"{mm128i_tmp}", "8"))
            self._MUL(f"{mm128i_tmp}", f"{inA}[3]", "poly", "0x1")
            self._ADD(f"{res}[1]", f"{res}[1]", self._LBYTESHIFT(f"{mm128i_tmp}", "8"))
            self._ADD(
                f"{mm128i_tmp}", f"{inA}[2]", self._RBYTESHIFT(f"{mm128i_tmp}", "8")
            )
            self._MUL(f"{mm128i_tmp}", f"{mm128i_tmp}", "poly", "0x0")
            self._ADD(f"{res}[0]", f"{res}[0]", f"{mm128i_tmp}")

    @override
    def carry_round(
        self, doublecarryover: bool = False, doublecarry: bool = False
    ) -> None:
        self._function_header(
            "int",
            "carry_round",
            [(f"{self.field_elem_t}*", "res"), (f"{self.dfield_elem_t}*", "a")],
        )
        self._startBody()
        self._declare_var("__m128i", f"tt[{self.numvecs}]", "{0}")
        if self.cmulReduction and self.fieldsize in [64, 128, 192, 256]:
            self._cmul_carry_round(res="res->val", inA="a->val", mm128i_tmp="tt[0]")
        else:
            self._declare_var(self.dfield_elem_t, "t")
            self._shift_carry_round(
                res="res->val", inA="a->val", field_tmp="tt", dfield_tmp="t.val"
            )
        self._endBody()

        self._function_header(
            "int",
            "_carry_round",
            [(f"{self.field_elem_t}*", "res"), (f"{self.field_elem_t}*", "a")],
        )
        self._startBody()
        self._IF("res != a")
        self._CALL("memcpy", ["res", "a", f"sizeof({self.field_elem_t})"])
        self._ENDIF()
        self._endBody()

    @override
    def square(self, doublecarryover: bool = False, doublecarry: bool = False) -> None:
        self._function_header(
            "int",
            "field_sqr",
            [(f"{self.field_elem_t}*", "res"), (f"const {self.field_elem_t}*", "a")],
        )
        self._startBody()
        self._declare_var(self.dfield_elem_t, "aa")
        self._square(res="aa.val", inA="a->val")
        if self.cmulReduction and self.fieldsize in [64, 128, 102, 256]:
            self._declare_var("__m128i", f"tt", "{0}")
            self._cmul_carry_round(res="res->val", inA="aa.val", mm128i_tmp="tt")
        else:
            self._declare_var("__m128i", f"tt[{self.numvecs}]", "{0}")
            self._declare_var(self.dfield_elem_t, "t")
            self._shift_carry_round(
                res="res->val", inA="aa.val", field_tmp="tt", dfield_tmp="t.val"
            )
        self._endBody()

    def _square(self, res, inA) -> None:
        if self.numlimbs % 2 == 0:
            for i in range(self.numlimbs):
                self._coment(f"clmul(a[{i}], a[{i}])")
                self._MUL(
                    f"{res}[{i}]",
                    f"{inA}[{i//2}]",
                    f"{inA}[{i//2}]",
                    f"{(i % 2) + ((i % 2)<<4)}",
                )
        else:
            self._START_BLOCK()
            self._declare_var("__m128i", f"d[{max(2*(self.numvecs)-1,1)}]", "{0}")
            for i in range(self.numlimbs):
                self._coment(f"clmul(a[{i}], a[{i}])")
                self._MUL(
                    f"d[{i}]",
                    f"{inA}[{i//2}]",
                    f"{inA}[{i//2}]",
                    f"{(i % 2) + ((i % 2)<<4)}",
                )
            print(file=self.file)
            if self.numlimbs == 1:
                self._ASSIGN(
                    f"{res}[{0}]",
                    f"_mm_and_si128(_mm_set_epi64x(0, 0xFFFFFFFFFFFFFFFF), d[{0}])",
                )
                self._ASSIGN(f"{res}[{1}]", f"_mm_bsrli_si128(d[{0}], 8)")
            else:
                for i in range(self.numvecs - 1):
                    self._ASSIGN(f"{res}[{i}]", f"d[{i}]")
                self._ASSIGN(
                    f"{res}[{self.numvecs-1}]",
                    "_mm_and_si128(_mm_set_epi64x(0, 0xFFFFFFFFFFFFFFFF),"
                    + f" d[{self.numvecs-1}])",
                )
                for j in range(self.numvecs, 2 * self.numvecs - 1):
                    self._ASSIGN(
                        f"{res}[{j}]",
                        f"_mm_or_si128(_mm_bsrli_si128(d[{j-1}], 8),"
                        + f" _mm_bslli_si128(d[{j}], 8))",
                    )
                self._ASSIGN(
                    f"{res}[{2*self.numvecs-1}]",
                    f"_mm_bsrli_si128(d[{2*self.numvecs-2}], 8)",
                )
            self._END_BLOCK()

    @override
    def square_no_carry(self) -> None:
        self._function_header(
            "int",
            "field_sqr_no_carry",
            [(f"{self.dfield_elem_t}*", "res"), (f"const {self.field_elem_t}*", "a")],
        )
        self._startBody()
        self._square(res="res->val", inA="a->val")
        self._endBody()

    @override
    def define_types(self) -> None:
        print(f"typedef {self.int_t} baseint_t;", file=self.file)
        print(
            f"typedef struct int{self.polynomial[0]}_single {{ __m128i val[{self.numvecs}];}}"
            + f" {self.field_elem_t};",
            file=self.file,
        )
        print(
            f"typedef struct int{self.polynomial[0]}_double {{ __m128i val[{2*self.numvecs}];}}"
            + f" {self.dfield_elem_t};",
            file=self.file,
        )

    @override
    def includes(self) -> None:
        super().includes()
        print("#include <immintrin.h>", file=self.file)
        print("#include <string.h>", file=self.file)

    @override
    def define_constants(self) -> None:
        print(f"#define DOUBLE_WORDSIZE {2*self.wordsize}", file=self.file)
        print(f"#define LIMBMASK (((({self.int_t})1) << LIMBBITS) - 1)", file=self.file)
        print(
            f"#define LIMBMASK2 (((({self.int_t})1) << LIMBBITS2) - 1)", file=self.file
        )

    @override
    def field_addition_mixed(self) -> None:
        self._function_header(
            "int",
            "field_add_mix",
            [
                (f"{self.dfield_elem_t}*", "res"),
                (f"const {self.dfield_elem_t}*", "a"),
                (f"const {self.field_elem_t}*", "b"),
            ],
        )
        self._startBody()
        for i in range(0, self.numvecs):
            self._ADD(f"res->val[{i}]", f"a->val[{i}]", f"b->val[{i}]")
        for i in range(self.numvecs, 2 * self.numvecs):
            self._ASSIGN(f"res->val[{i}]", f"a->val[{i}]")
        self._endBody()

    @override
    def field_addition_reduce(self) -> None:
        self._function_header(
            "int",
            "field_add_reduce",
            [
                (f"{self.field_elem_t}*", "res"),
                (f"const {self.field_elem_t}*", "a"),
                (f"const {self.field_elem_t}*", "b"),
            ],
        )
        self._startBody()
        self._CALL("field_add", ["res", "a", "b"])
        self._endBody()

    @override
    def field_mul_reduce(self) -> None:
        self._function_header(
            "int",
            "field_mul_reduce",
            [
                (f"{self.field_elem_t}*", "res"),
                (f"const {self.field_elem_t}*", "a"),
                (f"const {self.field_elem_t}*", "b"),
            ],
        )
        self._startBody()
        self._CALL("field_mul", ["res", "a", "b"])
        self._endBody()

    @override
    def reduce(self) -> None:
        self._function_header(
            "int",
            "reduce",
            [(f"{self.field_elem_t}*", "res"), (f"const {self.field_elem_t}*", "a")],
        )
        self._startBody()
        self._IF("res != a")
        self._CALL("memcpy", ["res", "a", f"sizeof({self.field_elem_t})"])
        self._ENDIF()
        self._endBody()

    @override
    def square_reduce(self) -> None:
        self._function_header(
            "int",
            "field_sqr_reduce",
            [(f"{self.field_elem_t}*", "res"), (f"const {self.field_elem_t}*", "a")],
        )
        self._startBody()
        self._CALL("field_sqr", ["res", "a"])
        self._endBody()

    def unified_api(self) -> None:
        print(
            "#define GET_MACRO(_1,_2,_3,NAME,...) NAME",
            file=self.file,
        )
        print(
            "#define NOT_PRECOMPUTED(name) name",
            file=self.file,
        )
        print(
            "#define PRECOMPUTED(name) name",
            file=self.file,
        )
        print(
            "#define DECLARE_PC_ELEM(name) field_elem_t NOT_PRECOMPUTED(name);",
            file=self.file,
        )
        print(
            "#define DECLARE_PC_ELEM_ARRAY(name, size)\\\n"
            + "  field_elem_t NOT_PRECOMPUTED(name)[size]",
            file=self.file,
        )
        print(
            "#define UNPACK_PC_FIELD_ELEM_SINGLE(name,buff)\\\n"
            + "  unpack_field_elem(&NOT_PRECOMPUTED(name), (baseint_t *) buff);",
            file=self.file,
        )
        print(
            "#define UNPACK_PC_FIELD_ELEM_ARRAY(name,buff,index)\\\n"
            + "  unpack_field_elem(&NOT_PRECOMPUTED(name)[index], (baseint_t *) buff)",
            file=self.file,
        )
        print(
            "#define UNPACK_PC_FIELD_ELEM(...)\\\n"
            + "  GET_MACRO(__VA_ARGS__,UNPACK_PC_FIELD_ELEM_ARRAY,UNPACK_PC_FIELD_ELEM_SINGLE)(__VA_ARGS__)",
            file=self.file,
        )
        print(
            "#define UNPACK_AND_ENCODE_PC_KEY_SINGLE(name,buff)\\\n"
            + "  unpack_and_encode_key(&NOT_PRECOMPUTED(name), (baseint_t *) buff);",
            file=self.file,
        )
        print(
            "#define UNPACK_AND_ENCODE_PC_KEY_ARRAY(name,buff,index)\\\n"
            + "  unpack_and_encode_key(&NOT_PRECOMPUTED(name)[index], (baseint_t *) buff)",
            file=self.file,
        )
        print(
            "#define UNPACK_AND_ENCODE_PC_KEY(...)\\\n"
            + "  GET_MACRO(__VA_ARGS__,UNPACK_AND_ENCODE_PC_KEY_ARRAY,UNPACK_AND_ENCODE_PC_KEY_SINGLE)(__VA_ARGS__)",
            file=self.file,
        )
        print(
            "#define INIT_PC_KEY(dst, src)",
            file=self.file,
        )
        print(
            "#define FIELD_MUL_PC(dest,srcA,srcB)\\\n"
            + "  field_mul(dest, srcA, srcB);",
            file=self.file,
        )
        print(
            "#define FIELD_MUL_PC_NO_CARRY(dest,srcA,srcB)\\\n"
            + "  field_mul_no_carry(dest, srcA, srcB);",
            file=self.file,
        )
        print(
            "#define FIELD_MUL_PC_REDUCE(dest,srcA,srcB)\\\n"
            + "  field_mul_reduce(dest, srcA, srcB);",
            file=self.file,
        )
        print(
            "#define FIELD_SQR_PC(dest,srcA,srcB)\\\n"
            + "  field_sqr(dest, srcA, srcB);",
            file=self.file,
        )
        print(
            "#define FIELD_SQR_PC_NO_CARRY(dest,srcA,srcB)\\\n"
            + "  field_sqr_no_carry(dest, srcA, srcB);",
            file=self.file,
        )
        print(
            "#define FIELD_SQR_PC_REDUCE(dest,srcA,srcB)\\\n"
            + "  field_sqr_reduce(dest, srcA, srcB);",
            file=self.file,
        )

    @override
    def fieldmul_funs(
        self, doublecarryover: bool = False, doublecarry: bool = False
    ) -> None:
        super().fieldmul_funs(doublecarryover, doublecarry)
        print(file=self.file)
        self.unified_api()

    @override
    def _endBody(self, retVal="0") -> None:
        print(" " * self.tabdepth + f"return {retVal};", file=self.file)
        super()._endBody()
