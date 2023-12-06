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

import sys
from math import ceil
from typing import List, Optional
import itertools
from typing_extensions import override
from cpuinfo import get_cpu_info
from src.field_arithmetic.ArithmeticGenerator import ArithmeticGenerator


class BinaryFieldArithmeticGenerator(ArithmeticGenerator):
    def __init__(
        self,
        polynomial: List[int],
        limbbits,
        num_limbs,
        wordsize,
        tabdepth=0,
        encodingMSB: int = 0,
        lowerEncode=False,
        blocksize=16,
        keysize=16,
        lastOnlyEnc: bool = False,
        encodingMask: Optional[List[int]] = None,
        file=sys.stdout,
        explicitEncoding=True,
    ) -> None:
        super().__init__(
            limbbits,
            num_limbs,
            wordsize,
            tabdepth,
            encodingMSB,
            lowerEncode,
            blocksize,
            lastOnlyEnc,
            encodingMask,
            file,
            explicitEncoding,
        )
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
        self.numvecs: int = ceil(self.fieldsize / 128)
        self.numdlimbs: int = 2 * self.numlimbs
        self.keysize: int = keysize
        self.polynomial: List[int] = polynomial

    def _ADD(self, res, out, inp) -> None:
        print(f'{" "*self.tabdepth}{res} = _mm_xor_si128({out},{inp});', file=self.file)

    def _MUL(self, out, x, y, flag) -> None:
        print(
            f'{" "*self.tabdepth}{out} = _mm_clmulepi64_si128({x}, {y}, {flag});',
            file=self.file,
        )

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
    def pack_field_elem(self) -> None:
        self._function_header(
            "int",
            "pack_field_elem",
            [(f"{self.int_t}*", "res"), (f"{self.field_elem_t}*", "a")],
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
            [(f"{self.field_elem_t}*", "res"), (f"{self.int_t}*", "a")],
        )
        self._startBody()
        self._CALL("memcpy", ["res", "a", f"{self.blocksize}"])
        self._endBody()

    @override
    def unpack_key(self) -> None:
        self._function_header(
            "int",
            "unpack_key",
            [(f"{self.field_elem_t}*", "res"), (f"{self.int_t}*", "a")],
        )
        self._startBody()
        self._CALL("memcpy", ["res", "a", f"{self.keysize}"])
        self._endBody()

    @override
    def unpack_and_encode_field_elem(self) -> None:
        if self.explicitEncoding:
            super().unpack_and_encode_field_elem()
            return
        self._function_header(
            "int",
            "unpack_and_encode_field_elem",
            [(f"{self.field_elem_t}*", "res"), (f"{self.int_t}*", "a")],
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
                (f"{self.int_t}*", "a"),
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
                (f"{self.field_elem_t}*", "a"),
                (f"{self.field_elem_t}*", "b"),
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
                (f"{self.dfield_elem_t}*", "a"),
                (f"{self.dfield_elem_t}*", "b"),
            ],
        )
        self._startBody()
        for i in range(0, 2 * self.numvecs):
            self._ADD(f"res->val[{i}]", f"a->val[{i}]", f"b->val[{i}]")
        self._endBody()

    @override
    def field_mul(
        self, doublecarryover: bool = False, doublecarry: bool = False
    ) -> None:
        self._function_header(
            "int",
            "field_mul",
            [
                (f"{self.field_elem_t}*", "res"),
                (f"{self.field_elem_t}*", "a"),
                (f"{self.field_elem_t}*", "b"),
            ],
        )
        self._startBody()
        self._declare_var(self.dfield_elem_t, "aa")
        self._declare_var("__m128i", "acc", "{0}")
        self._declare_var("__m128i", f"d[{max(2*(self.numlimbs)-1,1)}]", "{0}")
        self._declare_var(self.dfield_elem_t, "t")
        self._declare_var("__m128i", f"tt[{self.numvecs}]", "{0}")
        for i, j in itertools.product(range(self.numlimbs), repeat=2):
            self._coment(f"clmul(a[{i}], b[{j}])")
            self._MUL(
                "acc", f"a->val[{i//2}]", f"b->val[{j//2}]", f"{(i % 2) + ((j % 2)<<4)}"
            )
            self._INC(f"d[{i+j}]", "acc")
        print(file=self.file)
        if self.numlimbs % 2 == 0:
            # Multiply
            for i in range(self.numlimbs):
                self._coment(
                    f"aa.val[{i}] = d[{2*i}] + (d[{2*i+1}] <B< 8) + (d[{2*i-1}] >B> 8)"
                )
                self._ASSIGN(f"aa.val[{i}]", f"d[{2*i}]")
                if (2 * i + 1) < 2 * self.numlimbs - 1:
                    self._ADD(
                        f"aa.val[{i}]",
                        f"aa.val[{i}]",
                        f"_mm_bslli_si128(d[{2*i+1}], 8)",
                    )
                if (2 * i - 1) > 0:
                    self._ADD(
                        f"aa.val[{i}]",
                        f"aa.val[{i}]",
                        f"_mm_bsrli_si128(d[{2*i-1}], 8)",
                    )
            # Carry/Reduce
            for i in range(self.numvecs):
                self._ADD(f"res->val[{i}]", f"aa.val[{i}]", f"aa.val[{i+self.numvecs}]")
            if self.numlimbs % 2 == 0:
                self._SWAP64(f"t.val[{0}]", f"aa.val[{self.numvecs}]")
                if self.numlimbs == 4:
                    self._UNPACKLO64(
                        f"t.val[{1}]", f"t.val[{0}]", f"aa.val[{self.numlimbs//2 + 1}]"
                    )
                    self._UNPACKHI64(
                        f"t.val[{0}]", f"aa.val[{self.numlimbs//2 + 1}]", f"t.val[{0}]"
                    )

                for k in range(self.numvecs):
                    for j in self.polynomial[1:-1]:
                        self._coment(f"* x^{j}")
                        self._ADD(
                            f"tt[{k}]",
                            f"tt[{k}]",
                            self._RBITSHIFT64(f"t.val[{k}]", 64 - j),
                        )
                    self._ADD(f"res->val[{k}]", f"res->val[{k}]", f"tt[{k}]")

                self._ADD(
                    f"aa.val[{self.numvecs}]",
                    f"aa.val[{self.numvecs}]",
                    f"_mm_and_si128(_mm_set_epi64x(0, 0xFFFFFFFFFFFFFFFF), tt[{0}])",
                )
                for j in self.polynomial[1:-1]:
                    self._coment(f"* x^{j}")
                    for i in range(0, self.numvecs):
                        self._ADD(
                            f"res->val[{i}]",
                            f"res->val[{i}]",
                            self._LBITSHIFT64(f"aa.val[{self.numvecs + i}]", j),
                        )
        else:
            # Multiply
            for i in range(self.numvecs):
                self._coment(
                    f"aa.val[{i}] = d[{2*i}] + (d[{2*i+1}] <B< 8) + (d[{2*i-1}] >B> 8)"
                )
                self._ASSIGN(f"aa.val[{i}]", f"d[{2*i}]")
                if (2 * i + 1) < 2 * self.numvecs - 1:
                    self._ADD(
                        f"aa.val[{i}]",
                        f"aa.val[{i}]",
                        f"_mm_bslli_si128(d[{2*i+1}], 8)",
                    )
                if (2 * i - 1) > 0:
                    self._ADD(
                        f"aa.val[{i}]",
                        f"aa.val[{i}]",
                        f"_mm_bsrli_si128(d[{2*i-1}], 8)",
                    )
            self._ASSIGN(
                f"aa.val[{self.numvecs-1}]",
                "_mm_and_si128(_mm_set_epi64x(0, 0xFFFFFFFFFFFFFFFF),"
                + f" aa.val[{self.numvecs-1}])",
            )
            for j in range(self.numvecs, 2 * self.numvecs - 1):
                i: int = 2 * j - 1
                self._coment(
                    f"aa.val[{j}] = d[{i}] + (d[{i+1}] <B< 8) + (d[{i-1}] >B> 8)"
                )
                self._ASSIGN(f"aa.val[{j}]", f"d[{i}]")
                if (i + 1) < 2 * self.numlimbs - 1:
                    self._ADD(
                        f"aa.val[{j}]", f"aa.val[{j}]", f"_mm_bslli_si128(d[{i+1}], 8)"
                    )
                if (i - 1) > 0:
                    self._ADD(
                        f"aa.val[{j}]", f"aa.val[{j}]", f"_mm_bsrli_si128(d[{i-1}], 8)"
                    )
            self._ASSIGN(
                f"aa.val[{2*self.numvecs-1}]",
                f"_mm_bsrli_si128(d[{4*self.numvecs-4}], 8)",
            )

            # Carry/Reduce
            for i in range(self.numvecs):
                self._ADD(f"res->val[{i}]", f"aa.val[{i}]", f"aa.val[{i+self.numvecs}]")
            if self.numlimbs == 1:
                self._ASSIGN(f"t.val[{0}]", f"aa.val[{self.numvecs}]")
            else:
                self._UNPACKHI64(
                    f"t.val[{1}]",
                    f"aa.val[{self.numvecs}]",
                    f"aa.val[{self.numvecs + 1}]",
                )
                self._UNPACKLO64(
                    f"t.val[{0}]",
                    f"aa.val[{self.numvecs + 1}]",
                    f"aa.val[{self.numvecs}]",
                )
            for k in range(self.numvecs):
                for j in self.polynomial[1:-1]:
                    self._coment(f"* x^{j}")
                    self._ADD(
                        f"tt[{k}]", f"tt[{k}]", self._RBITSHIFT64(f"t.val[{k}]", 64 - j)
                    )
                self._ADD(f"res->val[{k}]", f"res->val[{k}]", f"tt[{k}]")

            self._ADD(
                f"aa.val[{self.numvecs}]",
                f"aa.val[{self.numvecs}]",
                f"_mm_and_si128(_mm_set_epi64x(0, 0xFFFFFFFFFFFFFFFF), tt[{0}])",
            )
            for j in self.polynomial[1:-1]:
                self._coment(f"* x^{j}")
                for i in range(0, self.numvecs):
                    self._ADD(
                        f"res->val[{i}]",
                        f"res->val[{i}]",
                        self._LBITSHIFT64(f"aa.val[{self.numvecs + i}]", j),
                    )
        self._endBody()

    @override
    def field_mul_no_carry(self) -> None:
        self._function_header(
            "int",
            "field_mul_no_carry",
            [
                (f"{self.dfield_elem_t}*", "res"),
                (f"{self.field_elem_t}*", "a"),
                (f"{self.field_elem_t}*", "b"),
            ],
        )
        self._startBody()
        self._declare_var("__m128i", "acc", "{0}")
        self._declare_var("__m128i", f"d[{max(2*(self.numlimbs)-1,1)}]", "{0}")
        for i, j in itertools.product(range(self.numlimbs), repeat=2):
            self._coment(f"clmul(a[{i}], b[{j}])")
            self._MUL(
                "acc", f"a->val[{i//2}]", f"b->val[{j//2}]", f"{(i % 2) + ((j % 2)<<4)}"
            )
            self._INC(f"d[{i+j}]", "acc")
        print(file=self.file)
        if self.numlimbs % 2 == 0:
            for i in range(self.numlimbs):
                self._coment(
                    f"res->val[{i}] = d[{2*i}] + (d[{2*i+1}] <B< 8) + (d[{2*i-1}] >B> 8)"
                )
                self._ASSIGN(f"res->val[{i}]", f"d[{2*i}]")
                if (2 * i + 1) < 2 * self.numlimbs - 1:
                    self._ADD(
                        f"res->val[{i}]",
                        f"res->val[{i}]",
                        f"_mm_bslli_si128(d[{2*i+1}], 8)",
                    )
                if (2 * i - 1) > 0:
                    self._ADD(
                        f"res->val[{i}]",
                        f"res->val[{i}]",
                        f"_mm_bsrli_si128(d[{2*i-1}], 8)",
                    )
        else:
            for i in range(self.numvecs):
                self._coment(
                    f"res->val[{i}] = d[{2*i}] + (d[{2*i+1}] <B< 8) + (d[{2*i-1}] >B> 8)"
                )
                self._ASSIGN(f"res->val[{i}]", f"d[{2*i}]")
                if (2 * i + 1) < 2 * self.numvecs - 1:
                    self._ADD(
                        f"res->val[{i}]",
                        f"res->val[{i}]",
                        f"_mm_bslli_si128(d[{2*i+1}], 8)",
                    )
                if (2 * i - 1) > 0:
                    self._ADD(
                        f"res->val[{i}]",
                        f"res->val[{i}]",
                        f"_mm_bsrli_si128(d[{2*i-1}], 8)",
                    )
            self._ASSIGN(
                f"res->val[{self.numvecs-1}]",
                "_mm_and_si128(_mm_set_epi64x(0, 0xFFFFFFFFFFFFFFFF),"
                + f" res->val[{self.numvecs-1}])",
            )
            for j in range(self.numvecs, 2 * self.numvecs - 1):
                i: int = 2 * j - 1
                self._coment(
                    f"res->val[{j}] = d[{i}] + (d[{i+1}] <B< 8) + (d[{i-1}] >B> 8)"
                )
                self._ASSIGN(f"res->val[{j}]", f"d[{i}]")
                if (i + 1) < 2 * self.numlimbs - 1:
                    self._ADD(
                        f"res->val[{j}]",
                        f"res->val[{j}]",
                        f"_mm_bslli_si128(d[{i+1}], 8)",
                    )
                if (i - 1) > 0:
                    self._ADD(
                        f"res->val[{j}]",
                        f"res->val[{j}]",
                        f"_mm_bsrli_si128(d[{i-1}], 8)",
                    )
            self._ASSIGN(
                f"res->val[{2*self.numvecs-1}]",
                f"_mm_bsrli_si128(d[{4*self.numvecs-4}], 8)",
            )

        self._endBody()

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
        self._declare_var(self.dfield_elem_t, "t")
        self._declare_var("__m128i", f"tt[{self.numvecs}]", "{0}")

        for i in range(self.numvecs):
            self._ADD(f"res->val[{i}]", f"a->val[{i}]", f"a->val[{i+self.numvecs}]")
        if self.numlimbs % 2 == 0:
            self._SWAP64(f"t.val[{0}]", f"a->val[{self.numvecs}]")
            if self.numlimbs == 4:
                self._UNPACKLO64(
                    f"t.val[{1}]", f"t.val[{0}]", f"a->val[{self.numlimbs//2 + 1}]"
                )
                self._UNPACKHI64(
                    f"t.val[{0}]", f"a->val[{self.numlimbs//2 + 1}]", f"t.val[{0}]"
                )

            for k in range(self.numvecs):
                for j in self.polynomial[1:-1]:
                    self._coment(f"* x^{j}")
                    self._ADD(
                        f"tt[{k}]", f"tt[{k}]", self._RBITSHIFT64(f"t.val[{k}]", 64 - j)
                    )
                self._ADD(f"res->val[{k}]", f"res->val[{k}]", f"tt[{k}]")

            self._ADD(
                f"a->val[{self.numvecs}]",
                f"a->val[{self.numvecs}]",
                f"_mm_and_si128(_mm_set_epi64x(0, 0xFFFFFFFFFFFFFFFF), tt[{0}])",
            )
            for j in self.polynomial[1:-1]:
                self._coment(f"* x^{j}")
                for i in range(0, self.numvecs):
                    self._ADD(
                        f"res->val[{i}]",
                        f"res->val[{i}]",
                        self._LBITSHIFT64(f"a->val[{self.numvecs + i}]", j),
                    )
        else:
            if self.numlimbs == 1:
                self._ASSIGN(f"t.val[{0}]", f"a->val[{self.numvecs}]")
            else:
                self._UNPACKHI64(
                    f"t.val[{1}]",
                    f"a->val[{self.numvecs}]",
                    f"a->val[{self.numvecs + 1}]",
                )
                self._UNPACKLO64(
                    f"t.val[{0}]",
                    f"a->val[{self.numvecs + 1}]",
                    f"a->val[{self.numvecs}]",
                )
            for k in range(self.numvecs):
                for j in self.polynomial[1:-1]:
                    self._coment(f"* x^{j}")
                    self._ADD(
                        f"tt[{k}]", f"tt[{k}]", self._RBITSHIFT64(f"t.val[{k}]", 64 - j)
                    )
                self._ADD(f"res->val[{k}]", f"res->val[{k}]", f"tt[{k}]")

            self._ADD(
                f"a->val[{self.numvecs}]",
                f"a->val[{self.numvecs}]",
                f"_mm_and_si128(_mm_set_epi64x(0, 0xFFFFFFFFFFFFFFFF), tt[{0}])",
            )
            for j in self.polynomial[1:-1]:
                self._coment(f"* x^{j}")
                for i in range(0, self.numvecs):
                    self._ADD(
                        f"res->val[{i}]",
                        f"res->val[{i}]",
                        self._LBITSHIFT64(f"a->val[{self.numvecs + i}]", j),
                    )
        self._endBody()

        self._function_header(
            "int",
            "_carry_round",
            [(f"{self.field_elem_t}*", "res"), (f"{self.field_elem_t}*", "a")],
        )
        self._startBody()
        self._endBody()

    @override
    def square(self, doublecarryover: bool = False, doublecarry: bool = False) -> None:
        self._function_header(
            "int",
            "field_sqr",
            [(f"{self.field_elem_t}*", "res"), (f"{self.field_elem_t}*", "a")],
        )
        self._startBody()
        self._declare_var(self.dfield_elem_t, "aa")
        self._declare_var(self.dfield_elem_t, "t")
        self._declare_var("__m128i", f"tt[{self.numvecs}]", "{0}")
        if self.numlimbs % 2 == 0:
            for i in range(self.numlimbs):
                self._coment(f"clmul(a[{i}], a[{i}])")
                self._MUL(
                    f"aa.val[{i}]",
                    f"a->val[{i//2}]",
                    f"a->val[{i//2}]",
                    f"{(i % 2) + ((i % 2)<<4)}",
                )

            # Carry/Reduce
            for i in range(self.numvecs):
                self._ADD(f"res->val[{i}]", f"aa.val[{i}]", f"aa.val[{i+self.numvecs}]")
            if self.numlimbs % 2 == 0:
                self._SWAP64(f"t.val[{0}]", f"aa.val[{self.numvecs}]")
                if self.numlimbs == 4:
                    self._UNPACKLO64(
                        f"t.val[{1}]", f"t.val[{0}]", f"aa.val[{self.numlimbs//2 + 1}]"
                    )
                    self._UNPACKHI64(
                        f"t.val[{0}]", f"aa.val[{self.numlimbs//2 + 1}]", f"t.val[{0}]"
                    )

                for k in range(self.numvecs):
                    for j in self.polynomial[1:-1]:
                        self._coment(f"* x^{j}")
                        self._ADD(
                            f"tt[{k}]",
                            f"tt[{k}]",
                            self._RBITSHIFT64(f"t.val[{k}]", 64 - j),
                        )
                    self._ADD(f"res->val[{k}]", f"res->val[{k}]", f"tt[{k}]")

                self._ADD(
                    f"aa.val[{self.numvecs}]",
                    f"aa.val[{self.numvecs}]",
                    f"_mm_and_si128(_mm_set_epi64x(0, 0xFFFFFFFFFFFFFFFF), tt[{0}])",
                )
                for j in self.polynomial[1:-1]:
                    self._coment(f"* x^{j}")
                    for i in range(0, self.numvecs):
                        self._ADD(
                            f"res->val[{i}]",
                            f"res->val[{i}]",
                            self._LBITSHIFT64(f"aa.val[{self.numvecs + i}]", j),
                        )
        else:
            self._declare_var("__m128i", f"d[{max(2*(self.numvecs)-1,1)}]", "{0}")
            for i in range(self.numlimbs):
                self._coment(f"clmul(a[{i}], a[{i}])")
                self._MUL(
                    f"d[{i}]",
                    f"a->val[{i//2}]",
                    f"a->val[{i//2}]",
                    f"{(i % 2) + ((i % 2)<<4)}",
                )
            print(file=self.file)
            if self.numlimbs == 1:
                self._ASSIGN(
                    f"aa.val[{0}]",
                    f"_mm_and_si128(_mm_set_epi64x(0, 0xFFFFFFFFFFFFFFFF), d[{0}])",
                )
                self._ASSIGN(f"aa.val[{1}]", f"_mm_bsrli_si128(d[{0}], 8)")
            else:
                for i in range(self.numvecs - 1):
                    self._ASSIGN(f"aa.val[{i}]", f"d[{i}]")
                self._ASSIGN(
                    f"aa.val[{self.numvecs-1}]",
                    "_mm_and_si128(_mm_set_epi64x(0, 0xFFFFFFFFFFFFFFFF),"
                    + f" d[{self.numvecs-1}])",
                )
                for j in range(self.numvecs, 2 * self.numvecs - 1):
                    self._ASSIGN(
                        f"aa.val[{j}]",
                        f"_mm_or_si128(_mm_bsrli_si128(d[{j-1}], 8),"
                        + f" _mm_bslli_si128(d[{j}], 8))",
                    )
                self._ASSIGN(
                    f"aa.val[{2*self.numvecs-1}]",
                    f"_mm_bsrli_si128(d[{2*self.numvecs-2}], 8)",
                )

            # Carry/Reduce
            for i in range(self.numvecs):
                self._ADD(f"res->val[{i}]", f"aa.val[{i}]", f"aa.val[{i+self.numvecs}]")
            if self.numlimbs == 1:
                self._ASSIGN(f"t.val[{0}]", f"aa.val[{self.numvecs}]")
            else:
                self._UNPACKHI64(
                    f"t.val[{1}]",
                    f"aa.val[{self.numvecs}]",
                    f"aa.val[{self.numvecs + 1}]",
                )
                self._UNPACKLO64(
                    f"t.val[{0}]",
                    f"aa.val[{self.numvecs + 1}]",
                    f"aa.val[{self.numvecs}]",
                )
            for k in range(self.numvecs):
                for j in self.polynomial[1:-1]:
                    self._coment(f"* x^{j}")
                    self._ADD(
                        f"tt[{k}]", f"tt[{k}]", self._RBITSHIFT64(f"t.val[{k}]", 64 - j)
                    )
                self._ADD(f"res->val[{k}]", f"res->val[{k}]", f"tt[{k}]")

            self._ADD(
                f"aa.val[{self.numvecs}]",
                f"aa.val[{self.numvecs}]",
                f"_mm_and_si128(_mm_set_epi64x(0, 0xFFFFFFFFFFFFFFFF), tt[{0}])",
            )
            for j in self.polynomial[1:-1]:
                self._coment(f"* x^{j}")
                for i in range(0, self.numvecs):
                    self._ADD(
                        f"res->val[{i}]",
                        f"res->val[{i}]",
                        self._LBITSHIFT64(f"aa.val[{self.numvecs + i}]", j),
                    )
        self._endBody()

    @override
    def square_no_carry(self) -> None:
        self._function_header(
            "int",
            "field_sqr_no_carry",
            [(f"{self.dfield_elem_t}*", "res"), (f"{self.field_elem_t}*", "a")],
        )
        self._startBody()
        if self.numlimbs % 2 == 0:
            for i in range(self.numlimbs):
                self._coment(f"clmul(a[{i}], a[{i}])")
                self._MUL(
                    f"res->val[{i}]",
                    f"a->val[{i//2}]",
                    f"a->val[{i//2}]",
                    f"{(i % 2) + ((i % 2)<<4)}",
                )
        else:
            self._declare_var("__m128i", f"d[{max(2*(self.numvecs)-1,1)}]", "{0}")
            for i in range(self.numlimbs):
                self._coment(f"clmul(a[{i}], a[{i}])")
                self._MUL(
                    f"d[{i}]",
                    f"a->val[{i//2}]",
                    f"a->val[{i//2}]",
                    f"{(i % 2) + ((i % 2)<<4)}",
                )
            print(file=self.file)
            if self.numlimbs == 1:
                self._ASSIGN(
                    f"res->val[{0}]",
                    f"_mm_and_si128(_mm_set_epi64x(0, 0xFFFFFFFFFFFFFFFF), d[{0}])",
                )
                self._ASSIGN(f"res->val[{1}]", f"_mm_bsrli_si128(d[{0}], 8)")
            else:
                for i in range(self.numvecs - 1):
                    self._ASSIGN(f"res->val[{i}]", f"d[{i}]")
                self._ASSIGN(
                    f"res->val[{self.numvecs-1}]",
                    "_mm_and_si128(_mm_set_epi64x(0, 0xFFFFFFFFFFFFFFFF),"
                    + f" d[{self.numvecs-1}])",
                )
                for j in range(self.numvecs, 2 * self.numvecs - 1):
                    self._ASSIGN(
                        f"res->val[{j}]",
                        f"_mm_or_si128(_mm_bsrli_si128(d[{j-1}], 8),"
                        + f" _mm_bslli_si128(d[{j}], 8))",
                    )
                self._ASSIGN(
                    f"res->val[{2*self.numvecs-1}]",
                    f"_mm_bsrli_si128(d[{2*self.numvecs-2}], 8)",
                )
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
                (f"{self.dfield_elem_t}*", "a"),
                (f"{self.field_elem_t}*", "b"),
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
                (f"{self.field_elem_t}*", "a"),
                (f"{self.field_elem_t}*", "b"),
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
                (f"{self.field_elem_t}*", "a"),
                (f"{self.field_elem_t}*", "b"),
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
            [(f"{self.field_elem_t}*", "res"), (f"{self.field_elem_t}*", "a")],
        )
        self._startBody()
        self._endBody()

    @override
    def square_reduce(self) -> None:
        self._function_header(
            "int",
            "field_sqr_reduce",
            [(f"{self.field_elem_t}*", "res"), (f"{self.field_elem_t}*", "a")],
        )
        self._startBody()
        self._CALL("field_sqr", ["res", "a"])
        self._endBody()

    @override
    def _endBody(self) -> None:
        print("return 0;", file=self.file)
        super()._endBody()
