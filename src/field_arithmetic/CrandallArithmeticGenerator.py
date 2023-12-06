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
from collections import Counter
from math import ceil
from typing import List, FrozenSet, Optional, Tuple
from typing_extensions import override
from numpy import cumsum
from src.field_arithmetic.ArithmeticGenerator import ArithmeticGenerator


def _LO(inp, wordsize: int) -> str:
    return f"(uint{wordsize}_t) ({inp})"


def _AND(a, b) -> str:
    return f"({a}) & ({b})"


def _OR(a, b) -> str:
    return f"({a}) | ({b})"


class CrandallArithmeticGenerator(ArithmeticGenerator):
    def __init__(
        self,
        pi: int,
        delta: int,
        limbbits: List[int],
        num_limbs: int,
        wordsize: int,
        buffsize: int,
        tabdepth: int = 0,
        encodingMSB: int = 0,
        lowerEncode: bool = False,
        blocksize: int = 16,
        keysize: int = 16,
        lastOnlyEnc: bool = False,
        encodingMask: Optional[List[int]] = None,
        file=sys.stdout,
        explicitEncoding: bool = True,
        nocheck: bool = False,
        doublecarry: bool = False,
        doublecarryover: bool = False,
        doublecarry_temp: bool = False,
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
        self.pi: int = pi
        self.delta: int = delta
        self.keysize: int = keysize
        self.buffsize: int = buffsize
        self.nocheck: bool = nocheck
        self.doublecarry: bool = doublecarry
        self.doublecarryover: bool = doublecarryover
        self.doublecarry_temp: bool = doublecarry_temp

    @override
    def _CALL(
        self, f: str, args: List[str], OFLAG: Optional[str] | bool = "OFLAG"
    ) -> None:
        if self.nocheck:
            super()._CALL(f, args)
            return
        if isinstance(OFLAG, bool):
            print(
                f'{" "*self.tabdepth}{"OFLAG |= "if OFLAG else ""}{f}({", ".join(args)});',
                file=self.file,
            )
        else:
            print(
                f'{" "*self.tabdepth}{OFLAG+" |= "if OFLAG else ""}{f}({", ".join(args)});',
                file=self.file,
            )

    def _MUL(self, out, x, y, nocheck=False, OFLAG="OFLAG", long_return=True) -> None:
        if long_return:
            return_type: str = self.long_t
        else:
            return_type = self.int_t
        if not (self.nocheck or nocheck):
            print(
                f'{" "*self.tabdepth}{{ '
                + f"{return_type} overflow_check_result; "
                + f"if(__builtin_mul_overflow(({return_type}) {x},"
                + f"({return_type}) {y}, &overflow_check_result))"
                + '{printf("Integer overflow in %s:%d\\n", __FILE__, __LINE__);'
                + ("" if OFLAG is None else f"{OFLAG} |= 1;")
                + "}}",
                file=self.file,
            )
        print(
            f'{" "*self.tabdepth}{out} = (({return_type}) {x} * {y});',
            file=self.file,
        )

    def _INC(self, out, inp, out_type, nocheck=False, OFLAG="OFLAG") -> None:
        if not (self.nocheck or nocheck):
            print(
                f'{" "*self.tabdepth}{{ '
                + f"{out_type} overflow_check_result; "
                + f"if(__builtin_add_overflow(({out_type}) {out},"
                + f"({out_type}) {inp}, &overflow_check_result))"
                + '{printf("Integer overflow in %s:%d\\n", __FILE__, __LINE__);'
                + ("" if OFLAG is None else f"{OFLAG} |= 1;")
                + "}}",
                file=self.file,
            )
        print(f'{" "*self.tabdepth}{out} += {inp};', file=self.file)

    def _INCLO(self, out, inp, out_type, nocheck=False, OFLAG="OFLAG") -> None:
        if not (self.nocheck or nocheck):
            print(
                f'{" "*self.tabdepth}{{ '
                + f"{out_type} overflow_check_result; "
                + f"if(__builtin_add_overflow(({out_type}) {out},"
                + f"({out_type}) {inp}, &overflow_check_result))"
                + '{printf("Integer overflow in %s:%d\\n", __FILE__, __LINE__);'
                + ("" if OFLAG is None else f"{OFLAG} |= 1;")
                + "}}",
                file=self.file,
            )
        print(f'{" "*self.tabdepth}{out} += {inp};', file=self.file)

    def _ADD(self, res, out, inp, res_type, nocheck=False, OFLAG="OFLAG") -> None:
        if not (self.nocheck or nocheck):
            print(
                f'{" "*self.tabdepth}{{ '
                + f"{res_type} overflow_check_result; "
                + f"if(__builtin_add_overflow(({res_type}) {out},"
                + f"({res_type}) {inp}, &overflow_check_result))"
                + '{printf("Integer overflow in %s:%d\\n", __FILE__, __LINE__);'
                + ("" if OFLAG is None else f"{OFLAG} |= 1;")
                + "}}",
                file=self.file,
            )
        print(f'{" "*self.tabdepth}{res} = {out} + {inp};', file=self.file)

    def _ADDLO(self, res, out, inp, res_type, nocheck=False, OFLAG="OFLAG") -> None:
        if not (self.nocheck or nocheck):
            print(
                f'{" "*self.tabdepth}{{ '
                + f"{res_type} overflow_check_result; "
                + f"if(__builtin_add_overflow(({res_type}) {out},"
                + f"({res_type}) {inp}, &overflow_check_result))"
                + '{printf("Integer overflow in %s:%d\\n", __FILE__, __LINE__);'
                + ("" if OFLAG is None else f"{OFLAG} |= 1;")
                + "}}",
                file=self.file,
            )
        print(f'{" "*self.tabdepth}{res} = {out} + {inp};', file=self.file)

    @override
    def pack_field_elem(self) -> None:
        self._function_header(
            "int",
            "pack_field_elem",
            [(f"{self.int_t}*", "res"), (f"{self.field_elem_t}*", "a")],
        )
        self._startBody(nocheck=True)
        j: int = 0
        self._ASSIGN(f"res[{j}]", 0)
        filled: int = 0
        packbits: List[int] = [self.wordsize] * (self.pi // self.wordsize) + [
            self.pi - self.wordsize * (self.pi // self.wordsize)
        ]
        for i, bits in enumerate(self.limbbits):
            taken: int = 0
            while taken < bits:
                self._OREQ(
                    f"res[{j}]",
                    self._SHL_exp(self._SHR_exp(f"a->val[{i}]", taken), filled),
                )
                filled += bits - taken
                taken = bits - taken - filled + self.wordsize
                if filled >= packbits[j]:
                    j += 1
                    if j >= ceil(self.pi / self.wordsize):
                        break
                    self._ASSIGN(f"res[{j}]", 0)
                    filled = 0
        self._endBody(nocheck=True)

    def limbmask(self, bits) -> str:
        return f'({self._SHL_exp(f"({self.int_t}) 1", bits)} - 1)'

    @override
    def unpack_key(self) -> None:
        self._function_header(
            "int",
            "unpack_key",
            [(f"{self.field_elem_t}*", "res"), (f"{self.int_t}*", "a")],
        )
        self._startBody(nocheck=True)
        j: int = 0
        taken: int = 0
        for i, bits in enumerate(self.limbbits):
            self._ASSIGN(f"res->val[{i}]", "0")
            filled: int = 0
            self._OREQ(
                f"res->val[{i}]",
                _AND(self._SHR_exp(f"a[{j}]", taken), self.limbmask(bits)),
            )
            filled += min([self.wordsize - taken, bits])
            taken += min([self.wordsize - taken, bits])
            if taken == self.wordsize:
                j += 1
            while (
                filled < bits
                and j < ceil(self.pi / self.wordsize)
                and j < ceil((self.keysize * 8) / self.wordsize)
            ):
                self._OREQ(
                    f"res->val[{i}]",
                    _AND(self._SHL_exp(f"a[{j}]", filled), self.limbmask(bits)),
                )
                taken = bits - filled
                filled += bits - filled
                if taken >= self.wordsize:
                    j += 1
            if (i == len(self.limbbits) - 1) and (self.keysize * 8) % self.wordsize:
                encshift: int = (self.keysize * 8) - sum(self.limbbits[:-1])
                mask = (1 << encshift) - 1
                self._ANDEQ(f"res->val[{i}]", f"{hex(min(mask, 2**(self.wordsize)-1))}")
            if j >= ceil(self.pi / self.wordsize) or j >= ceil(
                (self.keysize * 8) / self.wordsize
            ):
                break
        self._endBody(nocheck=True)

    @override
    def unpack_field_elem(self) -> None:
        self._function_header(
            "int",
            "unpack_field_elem",
            [(f"{self.field_elem_t}*", "res"), (f"{self.int_t}*", "a")],
        )
        self._startBody(nocheck=True)
        j: int = 0
        taken: int = 0
        for i, bits in enumerate(self.limbbits):
            self._ASSIGN(f"res->val[{i}]", "0")
            filled: int = 0
            self._OREQ(
                f"res->val[{i}]",
                _AND(self._SHR_exp(f"a[{j}]", taken), self.limbmask(bits)),
            )
            filled += min([self.wordsize - taken, bits])
            taken += min([self.wordsize - taken, bits])
            if taken == self.wordsize:
                j += 1
            while (
                filled < bits
                and j < ceil(self.pi / self.wordsize)
                and j < ceil((self.buffsize * 8) / self.wordsize)
            ):
                self._OREQ(
                    f"res->val[{i}]",
                    _AND(self._SHL_exp(f"a[{j}]", filled), self.limbmask(bits)),
                )
                taken = bits - filled
                filled += bits - filled
                if taken >= self.wordsize:
                    j += 1
            if (i == len(self.limbbits) - 1) and (self.buffsize * 8) % self.wordsize:
                encshift: int = (self.buffsize * 8) - sum(self.limbbits[:-1])
                mask = (1 << encshift) - 1
                self._ANDEQ(f"res->val[{i}]", f"{hex(mask)}")
            if j >= ceil(self.pi / self.wordsize) or j >= ceil(
                (self.buffsize * 8) / self.wordsize
            ):
                break
        self._endBody(nocheck=True)

    @override
    def unpack_and_encode_field_elem(self) -> None:
        if self.explicitEncoding:
            nocheck = self.nocheck
            self.nocheck = True
            super().unpack_and_encode_field_elem()
            self.nocheck = nocheck
            return
        self._function_header(
            "int",
            "unpack_and_encode_field_elem",
            [(f"{self.field_elem_t}*", "res"), (f"{self.int_t}*", "a")],
        )
        self._startBody(nocheck=True)
        j: int = 0
        taken: int = 0
        encshift: int = (self.blocksize * 8) - sum(self.limbbits[:-1])
        if self.lowerEncode and not self.lastOnlyEnc:
            encshift += self.encodingMSB.bit_length()
        for i, bits in enumerate(self.limbbits):
            encBits: int = 0
            filled: int = 0
            self._ASSIGN(f"res->val[{i}]", "0")
            if self.lowerEncode and i == 0 and not self.lastOnlyEnc:
                encBits = self.encodingMSB.bit_length()
                self._OREQ("res->val[0]", hex(self.encodingMSB))
            self._OREQ(
                f"res->val[{i}]",
                _AND(
                    self._SHL_exp(self._SHR_exp(f"a[{j}]", taken), encBits),
                    self.limbmask(bits),
                ),
            )
            filled += min([self.wordsize - taken, bits]) + encBits
            taken += min([self.wordsize - taken, bits]) - encBits
            if taken == self.wordsize:
                j += 1
            while (
                filled < bits
                and j < ceil(self.pi / self.wordsize)
                and j < ceil((self.buffsize * 8) / self.wordsize)
                and j < ceil((8 * self.blocksize) / self.wordsize)
            ):
                self._OREQ(
                    f"res->val[{i}]",
                    _AND(self._SHL_exp(f"a[{j}]", filled), self.limbmask(bits)),
                )
                taken = bits - filled
                filled += bits - filled
                if taken >= self.wordsize:
                    j += 1
            mask = self.encodingMask[i]
            if (i == len(self.limbbits) - 1) and (self.blocksize * 8) % self.wordsize:
                mask &= (1 << encshift) - 1
            self._ANDEQ(f"res->val[{i}]", f"{hex(mask)}")
            if (
                j >= ceil(self.pi / self.wordsize)
                or j >= ceil((self.buffsize * 8) / self.wordsize)
                or j >= ceil((8 * self.blocksize) / self.wordsize)
            ):
                break
        if not self.lowerEncode and not self.lastOnlyEnc:
            self._OREQ(
                f"res->val[{self.numlimbs-1}]",
                self._SHL_exp(f"({self.int_t}){hex(self.encodingMSB)}", encshift),
            )
        self._endBody(nocheck=True)

    @override
    def unpack_and_encode_last_field_elem(self) -> None:
        if self.explicitEncoding:
            nocheck = self.nocheck
            self.nocheck = True
            super().unpack_and_encode_last_field_elem()
            self.nocheck = nocheck
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
        self._startBody(nocheck=True)
        encshift: int = (self.blocksize * 8) - sum(self.limbbits[:-1])
        if self.lowerEncode:
            encshift += self.encodingMSB.bit_length()
        self._declare_var(f"{self.int_t}", "tmp[BUFFSIZE]", "{0}")
        self._CALL("memcpy", ["tmp", "a", "size"], OFLAG=None)
        if not self.lowerEncode:
            self._IF(f"size < {self.blocksize}")
            self._ASSIGN("((uint8_t*)tmp)[size]", f"{hex(self.encodingMSB)}")
            self._ENDIF()

        j: int = 0
        taken: int = 0
        for i, bits in enumerate(self.limbbits):
            encBits: int = 0
            filled: int = 0
            self._ASSIGN(f"res->val[{i}]", "0")
            if self.lowerEncode and i == 0:
                encBits = self.encodingMSB.bit_length()
                self._OREQ("res->val[0]", hex(self.encodingMSB))
            filled: int = 0
            self._OREQ(
                f"res->val[{i}]",
                _AND(
                    self._SHL_exp(self._SHR_exp(f"tmp[{j}]", taken), encBits),
                    self.limbmask(bits),
                ),
            )
            filled += min([self.wordsize - taken, bits]) + encBits
            taken += min([self.wordsize - taken, bits]) - encBits
            if taken == self.wordsize:
                j += 1
            while (
                filled < bits
                and j < ceil(self.pi / self.wordsize)
                and j < ceil((self.buffsize * 8) / self.wordsize)
                and j < ceil((8 * self.blocksize) / self.wordsize)
            ):
                self._OREQ(
                    f"res->val[{i}]",
                    _AND(self._SHL_exp(f"tmp[{j}]", filled), self.limbmask(bits)),
                )
                taken = bits - filled
                filled += bits - filled
                if taken >= self.wordsize:
                    j += 1
            mask: int = self.encodingMask[i]
            if (i == len(self.limbbits) - 1) and (self.blocksize * 8) % self.wordsize:
                mask &= (1 << encshift) - 1
            self._ANDEQ(f"res->val[{i}]", f"{hex(mask)}")
            if (
                j >= ceil(self.pi / self.wordsize)
                or j >= ceil((self.buffsize * 8) / self.wordsize)
                or j >= ceil((8 * self.blocksize) / self.wordsize)
            ):
                break

        if not self.lowerEncode:
            self._IF(f"size == {self.blocksize}")
            self._OREQ(
                f"res->val[{self.numlimbs-1}]",
                self._SHL_exp(f"({self.int_t}){hex(self.encodingMSB)}", encshift),
            )
            self._ENDIF()
        self._endBody(nocheck=True)

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
        for i in range(0, self.numlimbs):
            self._ADD(
                f"res->val[{i}]", f"a->val[{i}]", f"b->val[{i}]", res_type=self.int_t
            )
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
        self._declare_var(self.field_elem_t, "tmp")
        self._CALL("field_add", ["&tmp", "a", "b"], OFLAG=not self.nocheck)
        self._CALL("_carry_round", ["&tmp", "&tmp"], OFLAG=not self.nocheck)
        self._CALL("reduce", ["res", "&tmp"], OFLAG=not self.nocheck)
        self._endBody()

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
        for i in range(0, self.numlimbs):
            self._ADD(
                f"res->val[{i}]", f"a->val[{i}]", f"b->val[{i}]", res_type=self.long_t
            )
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
        for i in range(0, self.numlimbs):
            self._ADD(
                f"res->val[{i}]", f"a->val[{i}]", f"b->val[{i}]", res_type=self.long_t
            )
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
        if doublecarry:
            carrysize: int = self.wordsize * 2
        else:
            carrysize = self.wordsize
        self._declare_var(f"{self.long_t}", "acc")
        self._declare_var(f"{self.long_t}", f"d[{self.numlimbs}]", "{0}")
        self._declare_var(f"uint{carrysize}_t", "c")
        if self.need_double_carry_temp():
            self._declare_var(f"{self.long_t}", "t")
        else:
            self._declare_var(f"{self.int_t}", "t")
        for k in range(0, self.numlimbs):
            for i in range(0, self.numlimbs):
                for j in range(0, self.numlimbs):
                    kk: int = sum(self.limbbits[0:i]) + sum(self.limbbits[0:j])
                    if kk == sum(self.limbbits[0:k]):
                        self._MUL("acc", f"a->val[{i}]", f"b->val[{j}]")
                        self._INC(f"d[{k}]", "acc", out_type=self.long_t)
                    elif (
                        len(
                            [
                                x
                                for x in cumsum([0] + self.limbbits)
                                if x <= (kk - self.pi)
                            ]
                        )
                        == k + 1
                    ):
                        self._MUL(
                            "t",
                            f"b->val[{j}]",
                            f"{self._SHL_exp(self.delta, kk-self.pi - sum(self.limbbits[0:k]))}",
                            long_return=self.need_double_carry_temp(),
                        )
                        self._MUL(
                            "acc",
                            f"a->val[{i}]",
                            "t",
                        )
                        self._INC(f"d[{k}]", "acc", out_type=self.long_t)
            print(file=self.file)
        print(file=self.file)
        for i in range(0, self.numlimbs - 1):
            self._SHR("c", f"d[{i}]", self.limbbits[i], carrysize)
            self._ASSIGN(
                f"res->val[{i}]",
                _AND(_LO(f"d[{i}]", self.wordsize), self.limbmask(self.limbbits[i])),
            )
            self._INCLO(f"d[{i+1}]", "c", out_type=self.long_t)
        self._SHR("c", f"d[{self.numlimbs-1}]", self.limbbits[-1], carrysize)
        self._ASSIGN(
            f"res->val[{self.numlimbs-1}]",
            _AND(
                _LO(f"d[{self.numlimbs-1}]", self.wordsize),
                self.limbmask(self.limbbits[-1]),
            ),
        )
        self._MUL("c", "c", self.delta, long_return=doublecarryover)
        if doublecarryover:
            self._ADD("d[0]", "res->val[0]", "c", res_type=self.long_t)
            self._ASSIGN("c", self._SHR_exp("d[0]", self.limbbits[0]))
            self._ASSIGN("res->val[0]", _AND("d[0]", self.limbmask(self.limbbits[0])))
        else:
            self._INC("res->val[0]", "c", out_type=self.int_t)
            self._ASSIGN("c", self._SHR_exp("res->val[0]", self.limbbits[0]))
            self._ASSIGN(
                "res->val[0]", _AND("res->val[0]", self.limbmask(self.limbbits[0]))
            )
        if self.numlimbs > 1:
            self._INC("res->val[1]", "c", out_type=self.int_t)
        else:
            self._INC("res->val[0]", "c", out_type=self.int_t)
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
        self._declare_var(f"{self.long_t}", "acc")
        if self.need_double_carry_temp():
            self._declare_var(f"{self.long_t}", "t")
        else:
            self._declare_var(f"{self.int_t}", "t")
        for k in range(0, self.numlimbs):
            self._ASSIGN(f"res->val[{k}]", "0")
            for i in range(0, self.numlimbs):
                for j in range(0, self.numlimbs):
                    kk: int = sum(self.limbbits[0:i]) + sum(self.limbbits[0:j])
                    if kk == sum(self.limbbits[0:k]):
                        self._MUL("acc", f"a->val[{i}]", f"b->val[{j}]")
                        self._INC(f"res->val[{k}]", "acc", out_type=self.long_t)
                    elif (
                        len(
                            [
                                x
                                for x in cumsum([0] + self.limbbits)
                                if x <= (kk - self.pi)
                            ]
                        )
                        == k + 1
                    ):
                        self._MUL(
                            "t",
                            f"b->val[{j}]",
                            f"{self._SHL_exp(self.delta, kk-self.pi - sum(self.limbbits[0:k]))}",
                            long_return=self.need_double_carry_temp(),
                        )
                        self._MUL(
                            "acc",
                            f"a->val[{i}]",
                            "t",
                        )
                        self._INC(f"res->val[{k}]", "acc", out_type=self.long_t)
            print(file=self.file)
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
        self._declare_var(self.field_elem_t, "tmp")
        self._CALL("field_mul", ["&tmp", "a", "b"], OFLAG=not self.nocheck)
        self._CALL("reduce", ["res", "&tmp"], OFLAG=not self.nocheck)
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
        if doublecarry:
            carrysize: int = self.wordsize * 2
        else:
            carrysize = self.wordsize
        self._declare_var(f"uint{carrysize}_t", "c")
        for i in range(0, self.numlimbs - 1):
            self._SHR("c", f"a->val[{i}]", self.limbbits[i], carrysize)
            self._ASSIGN(
                f"res->val[{i}]",
                _AND(
                    _LO(f"a->val[{i}]", self.wordsize), self.limbmask(self.limbbits[i])
                ),
            )
            self._INCLO(f"a->val[{i+1}]", "c", out_type=self.long_t)
        self._SHR("c", f"a->val[{self.numlimbs-1}]", self.limbbits[-1], carrysize)
        self._ASSIGN(
            f"res->val[{self.numlimbs-1}]",
            _AND(
                _LO(f"a->val[{self.numlimbs-1}]", self.wordsize),
                self.limbmask(self.limbbits[-1]),
            ),
        )
        self._MUL("c", "c", self.delta, long_return=doublecarryover)
        if doublecarryover:
            self._ADD("a->val[0]", "res->val[0]", "c", res_type=self.long_t)
            self._ASSIGN("c", self._SHR_exp("a->val[0]", self.limbbits[0]))
            self._ASSIGN(
                "res->val[0]", _AND("a->val[0]", self.limbmask(self.limbbits[0]))
            )
        else:
            self._INC("res->val[0]", "c", out_type=self.int_t)
            self._ASSIGN("c", self._SHR_exp("res->val[0]", self.limbbits[0]))
            self._ASSIGN(
                "res->val[0]", _AND("res->val[0]", self.limbmask(self.limbbits[0]))
            )
        if self.numlimbs > 1:
            self._INC("res->val[1]", "c", out_type=self.int_t)
        else:
            self._INC("res->val[0]", "c", out_type=self.int_t)
        self._endBody()

        self._function_header(
            "int",
            "_carry_round",
            [(f"{self.field_elem_t}*", "res"), (f"{self.field_elem_t}*", "a")],
        )
        self._startBody()
        self._declare_var("uint64_t", "c")
        for i in range(0, self.numlimbs - 1):
            self._SHR("c", f"a->val[{i}]", self.limbbits[i], self.wordsize)
            self._ASSIGN(
                f"res->val[{i}]",
                _AND(
                    _LO(f"a->val[{i}]", self.wordsize), self.limbmask(self.limbbits[i])
                ),
            )
            self._INCLO(f"a->val[{i+1}]", "c", out_type=self.int_t)
        self._SHR("c", f"a->val[{self.numlimbs-1}]", self.limbbits[-1], self.wordsize)
        self._ASSIGN(
            f"res->val[{self.numlimbs-1}]",
            _AND(
                _LO(f"a->val[{self.numlimbs-1}]", self.wordsize),
                self.limbmask(self.limbbits[-1]),
            ),
        )
        self._MUL("c", "c", self.delta, long_return=False)
        self._INC("res->val[0]", "c", out_type=self.int_t)
        self._ASSIGN("c", self._SHR_exp("res->val[0]", self.limbbits[0]))
        self._ASSIGN(
            "res->val[0]", _AND("res->val[0]", self.limbmask(self.limbbits[0]))
        )
        self._INC("res->val[1]", "c", out_type=self.int_t)
        self._endBody()

    @override
    def reduce(self) -> None:
        self._function_header(
            "int",
            "reduce",
            [(f"{self.field_elem_t}*", "res"), (f"{self.field_elem_t}*", "a")],
        )
        self._startBody()
        self._declare_var(self.field_elem_t, "t")
        if self.numlimbs > 1:
            self._declare_var("uint64_t", "c")
        self._declare_var("uint64_t", "mask")
        self._ADD("t.val[0]", "a->val[0]", f"{self.delta}", res_type=self.int_t)
        for i in range(self.numlimbs - 1):
            self._SHR("c", f"t.val[{i}]", self.limbbits[i], self.wordsize)
            self._ASSIGN(
                f"t.val[{i}]",
                _AND(
                    _LO(f"t.val[{i}]", self.wordsize), self.limbmask(self.limbbits[i])
                ),
            )
            self._ADD(f"t.val[{i+1}]", f"a->val[{i+1}]", "c", res_type=self.int_t)

        self._INC(
            f"t.val[{self.numlimbs-1}]",
            f'-{self._SHL_exp(f"(({self.int_t}) 1)", self.limbbits[self.numlimbs-1])}',
            out_type=self.int_t,
            nocheck=True,
        )
        self._SHR(
            "mask", f"t.val[{self.numlimbs-1}]", f"{self.wordsize - 1}", self.wordsize
        )
        self._INC("mask", "-1", out_type="uint64_t", nocheck=True)
        for i in range(self.numlimbs):
            self._ASSIGN(f"t.val[{i}]", _AND(f"t.val[{i}]", "mask"))
        self._ASSIGN("mask", "~mask")
        for i in range(self.numlimbs):
            self._ASSIGN(
                f"res->val[{i}]", _OR(_AND(f"a->val[{i}]", "mask"), f"t.val[{i}]")
            )
        self._endBody()

    @override
    def square(self, doublecarryover: bool = False, doublecarry: bool = False) -> None:
        self._function_header(
            "int",
            "field_sqr",
            [(f"{self.field_elem_t}*", "res"), (f"{self.field_elem_t}*", "a")],
        )
        self._startBody()
        self._CALL("field_mul", ["res", "a", "a"], OFLAG=not self.nocheck)
        # FIXME!
        # if doublecarry:
        #     carrysize: int = self.wordsize * 2
        # else:
        #     carrysize = self.wordsize
        # self._declare_var(f"uint{carrysize}_t", "c")
        # self._declare_var(f"{self.long_t}", "acc")
        # self._declare_var(f"{self.long_t}", f"d[{self.numlimbs}]", "{0}")
        # counters: List[Counter[FrozenSet[int]]] = []
        # for k in range(0, self.numlimbs):
        #     c: Counter[FrozenSet[int]] = Counter()
        #     for i in range(0, self.numlimbs):
        #         for j in range(0, self.numlimbs):
        #             kk = sum(self.limbbits[0:i]) + sum(self.limbbits[0:j])
        #             if kk == sum(self.limbbits[0:k]):
        #                 c.update({frozenset({i, j})})
        #             elif (
        #                 len(
        #                     [
        #                         x
        #                         for x in cumsum([0] + self.limbbits)
        #                         if x <= (kk - self.pi)
        #                     ]
        #                 )
        #                 == k + 1
        #             ):
        #                 c.update({frozenset({i, j})})
        #     counters.append(c)

        # for k, c in enumerate(counters):
        #     for s, cnt in c.items():
        #         if len(s) == 1:
        #             i: int = set(s).pop()
        #             j: int = i
        #         else:
        #             i, j = s
        #         kk: int = sum(self.limbbits[0:i]) + sum(self.limbbits[0:j])
        #         if kk == sum(self.limbbits[0:k]):
        #             self._MUL(
        #                 "acc",
        #                 f"a->val[{i}]",
        #                 f"a->val[{j}]" + (f" * {cnt}" if cnt > 1 else ""),
        #             )
        #             self._INC(f"d[{k}]", "acc")
        #         elif (
        #             len([x for x in cumsum([0] + self.limbbits) if x <= (kk - self.pi)])
        #             == k + 1
        #         ):
        #             self._MUL(
        #                 "acc",
        #                 f"a->val[{i}]",
        #                 f"a->val[{j}]"
        #                 + (f" * {cnt} " if cnt > 1 else " ")
        #                 + f"* {self._SHL_exp(self.delta, kk-self.pi - sum(self.limbbits[0:k]))}",
        #             )
        #             self._INC(f"d[{k}]", "acc")
        #     print(file=self.file)
        # print(file=self.file)
        # for i in range(0, self.numlimbs - 1):
        #     self._SHR("c", f"d[{i}]", self.limbbits[i], carrysize)
        #     self._ASSIGN(
        #         f"res->val[{i}]",
        #         _AND(_LO(f"d[{i}]", self.wordsize), self.limbmask(self.limbbits[i])),
        #     )
        #     self._INCLO(f"d[{i+1}]", "c")
        # self._SHR("c", f"d[{self.numlimbs-1}]", self.limbbits[-1], carrysize)
        # self._ASSIGN(
        #     f"res->val[{self.numlimbs-1}]",
        #     _AND(
        #         _LO(f"d[{self.numlimbs-1}]", self.wordsize),
        #         self.limbmask(self.limbbits[-1]),
        #     ),
        # )
        # self._MUL("c", "c", self.delta, long_return=doublecarryover)
        # if doublecarryover:
        #     self._ADD("d[0]", "res->val[0]", "c")
        #     self._ASSIGN("c", self._SHR_exp("d[0]", self.limbbits[0]))
        #     self._ASSIGN("res->val[0]", _AND("d[0]", self.limbmask(self.limbbits[0])))
        # else:
        #     self._INC("res->val[0]", "c")
        #     self._ASSIGN("c", self._SHR_exp("res->val[0]", self.limbbits[0]))
        #     self._ASSIGN(
        #         "res->val[0]", _AND("res->val[0]", self.limbmask(self.limbbits[0]))
        #     )
        # if self.numlimbs > 1:
        #     self._INC("res->val[1]", "c")
        # else:
        #     self._INC("res->val[0]", "c")
        self._endBody()

    @override
    def square_reduce(self) -> None:
        self._function_header(
            "int",
            "field_sqr_reduce",
            [(f"{self.field_elem_t}*", "res"), (f"{self.field_elem_t}*", "a")],
        )
        self._startBody()
        self._declare_var(self.field_elem_t, "tmp")
        self._CALL("field_sqr", ["&tmp", "a"], OFLAG=not self.nocheck)
        self._CALL("reduce", ["res", "&tmp"], OFLAG=not self.nocheck)
        self._endBody()

    @override
    def square_no_carry(self) -> None:
        self._function_header(
            "int",
            "field_sqr_no_carry",
            [(f"{self.dfield_elem_t}*", "res"), (f"{self.field_elem_t}*", "a")],
        )
        self._startBody()
        self._CALL("field_mul_no_carry", ["res", "a", "a"], OFLAG=not self.nocheck)
        # FIXME!
        # self._declare_var(f"{self.long_t}", "acc")
        # counters: List[Counter[FrozenSet[int]]] = []
        # for k in range(0, self.numlimbs):
        #     c: Counter[FrozenSet[int]] = Counter()
        #     for i in range(0, self.numlimbs):
        #         for j in range(0, self.numlimbs):
        #             kk: int = sum(self.limbbits[0:i]) + sum(self.limbbits[0:j])
        #             if kk == sum(self.limbbits[0:k]):
        #                 c.update({frozenset({i, j})})
        #             elif (
        #                 len(
        #                     [
        #                         x
        #                         for x in cumsum([0] + self.limbbits)
        #                         if x <= (kk - self.pi)
        #                     ]
        #                 )
        #                 == k + 1
        #             ):
        #                 c.update({frozenset({i, j})})
        #     counters.append(c)

        # for k, c in enumerate(counters):
        #     self._ASSIGN(f"res->val[{k}]", "0")
        #     for s, cnt in c.items():
        #         if len(s) == 1:
        #             i: int = set(s).pop()
        #             j: int = i
        #         else:
        #             i, j = s
        #         kk = sum(self.limbbits[0:i]) + sum(self.limbbits[0:j])
        #         if kk == sum(self.limbbits[0:k]):
        #             self._MUL(
        #                 "acc",
        #                 f"a->val[{i}]",
        #                 f"a->val[{j}]" + (f" * {cnt}" if cnt > 1 else ""),
        #             )
        #             self._INC(f"res->val[{k}]", "acc")
        #         elif (
        #             len([x for x in cumsum([0] + self.limbbits) if x <= (kk - self.pi)])
        #             == k + 1
        #         ):
        #             self._MUL(
        #                 "acc",
        #                 f"a->val[{i}]",
        #                 f"a->val[{j}]"
        #                 + (f" * {cnt} " if cnt > 1 else " ")
        #                 + f"* {self._SHL_exp(self.delta, kk-self.pi - sum(self.limbbits[0:k]))}",
        #             )
        #             self._INC(f"res->val[{k}]", "acc")
        #     print(file=self.file)
        self._endBody()

    @override
    def need_doublecarry(self) -> Tuple[bool, bool]:
        l: int = self.numlimbs
        lb: int = self.limbbits[0]
        lp: int = self.limbbits[-1]
        c: int = 0
        doublecarry: bool = False
        doublecarryover: bool = False
        for ii in range(0, l - 2):
            i: int = ii + 1
            b: int = (
                i * (2 ** (2 * lb) - 2 ** (lb + 1) + 1)
                + (
                    2 * (2 ** (lb + lp) - 2 ** (lb) - 2 ** (lp) + 1)
                    + (l - i - 2) * (2 ** (2 * lb) - 2 ** (lb + 1) + 1)
                )
                * self.delta
                * 2 ** (lb - lp)
                + c
            )
            if (b >> lb) >= 2**self.wordsize:
                doublecarry |= True
            c = b >> lb
        # i = l-1
        b = (
            (l - 1) * (2 ** (2 * lb) - 2 ** (lb + 1) + 1)
            + (2 ** (2 * lb) - 2 ** (lb + 1) + 1)
            + c
        )
        if (b >> lb) >= 2**self.wordsize:
            doublecarry |= True
        c = b >> lb
        # i = l
        b = (
            (l - 1) * (2 ** (2 * lb) - 2 ** (lb + 1) + 1)
            + 2 * (2 ** (lb + lp) - 2 ** (lb) - 2 ** (lp) + 1)
            + c
        )
        if (b >> lb) >= 2**self.wordsize:
            doublecarry |= True
        c = b >> lp

        b = c * self.delta + 2**lb
        if b >= 2**self.wordsize:
            doublecarryover |= True
        return doublecarry or self.doublecarry, doublecarryover or self.doublecarryover

    def need_double_carry_temp(self) -> bool:
        return (
            2 ** max(self.limbbits)
            * self.delta
            * 2 ** (max(self.limbbits) - min(self.limbbits))
            >= 2**self.wordsize
        ) or self.doublecarry_temp

    @override
    def need_doublecarryover(self) -> bool:
        _, doublecarryover = self.need_doublecarry()
        return doublecarryover or self.doublecarryover

    @override
    def includes(self):
        super().includes()
        print("#include <stdio.h>", file=self.file)
        print("#include <stdlib.h>", file=self.file)
        print("#include <execinfo.h>", file=self.file)
        print("#include <assert.h>", file=self.file)

    @override
    def define_types(self) -> None:
        print("typedef unsigned __int128 uint128_t;", file=self.file)
        print(f"typedef {self.int_t} baseint_t;", file=self.file)
        print(
            f"typedef struct int{self.pi}{self.delta}_single"
            + f" {{{self.int_t} val[{self.numlimbs}];}} {self.field_elem_t};",
            file=self.file,
        )
        print(
            f"typedef struct int{self.pi}{self.delta}_double"
            + f" {{{self.long_t} val[{self.numlimbs}];}} {self.dfield_elem_t};",
            file=self.file,
        )

    @override
    def define_constants(self) -> None:
        print(f"#define PI {self.pi}", file=self.file)
        print(f"#define DELTA {self.delta}", file=self.file)
        print(f"#define DOUBLE_WORDSIZE {2*self.wordsize}", file=self.file)
        print(f"#define LIMBMASK (((({self.int_t})1) << LIMBBITS) - 1)", file=self.file)
        print(
            f"#define LIMBMASK2 (((({self.int_t})1) << LIMBBITS2) - 1)", file=self.file
        )

    @override
    def _startBody(self, nocheck=False) -> None:
        super()._startBody()
        if not (self.nocheck or nocheck):
            self._declare_var("int", "OFLAG", 0)

    @override
    def _endBody(self, nocheck=False) -> None:
        if not (self.nocheck or nocheck):
            print(" " * self.tabdepth + "return OFLAG;", file=self.file)
        else:
            print(" " * self.tabdepth + "return 0;", file=self.file)
        super()._endBody()

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
    def footer(self) -> None:
        if not self.nocheck:
            for fun in self.funs:
                print(
                    f"#define {fun}(...) ({{if({fun}(__VA_ARGS__))"
                    + '{printf("Integer overflow in %s:%d\\n", __FILE__, __LINE__);}})',
                    file=self.file,
                )
