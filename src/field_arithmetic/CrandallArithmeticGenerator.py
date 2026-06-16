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
        buffsize: int,
        nocheck: bool = False,
        doublecarry: bool = False,
        doublecarryover: bool = False,
        doublecarry_temp: bool = False,
        *args,
        **kwargs,
    ) -> None:
        super().__init__(*args, **kwargs)
        self.pi: int = pi
        self.delta: int = delta
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
            [(f"{self.int_t}*", "res"), (f"const {self.field_elem_t}*", "a")],
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
    def field_elem_get_one(self) -> None:
        self._function_header(
            f"{self.field_elem_t}",
            "field_elem_get_one",
            [],
            nocheck=True,
        )
        self._startBody(nocheck=True)
        retVal = f"({self.field_elem_t})" + "{{1" + ", 0" * (self.numlimbs - 1) + "}}"
        self._endBody(nocheck=True, retVal=retVal)

    @override
    def unpack_key(self) -> None:
        self._function_header(
            "int",
            "unpack_key",
            [(f"{self.field_elem_t}*", "res"), (f"const {self.int_t}*", "a")],
        )
        self._startBody(nocheck=True)
        self._declare_var(name="key", typ="const uint8_t *const", val="(uint8_t *) a")
        taken: int = 0
        available_bytes = self.buffsize
        pos = 0
        pos_byte = 0
        for i, bits in enumerate(self.limbbits):
            pos_byte = pos // 8
            taken = pos % 8
            available_bytes = self.buffsize - pos_byte
            if available_bytes < self.wordbytes:
                pos_byte = self.buffsize - self.wordbytes
                taken += (self.wordbytes - available_bytes) * 8
            key_limb = f"*((uint{self.wordsize}_t *)(key+{pos_byte}))"

            key_bits_exp = self._SHR_exp(key_limb, taken)

            available_bits = self.wordsize - taken
            if available_bits < bits and available_bytes > self.wordbytes:
                key_limb = (
                    f"((uint{self.wordsize}_t) (key[{pos_byte + self.wordsize//8}]))"
                )
                key_bits_exp = _OR(
                    key_bits_exp, self._SHL_exp(key_limb, available_bits)
                )

            limb_clamp = 2**bits - 1
            self._ASSIGN(
                f"res->val[{i}]",
                _AND(
                    key_bits_exp,
                    hex(limb_clamp),
                ),
            )
            pos += bits
        self._endBody(nocheck=True)

    # def get_bytes(self, available_bytes, index, var):
    #     if available_bytes * 8 >= self.wordsize:
    #         return f"{var}[{index}]"
    #     res = []
    #     base = f"&({var}[{index}])"
    #     bits = [int(b) for b in "{0:03b}".format(available_bytes)]
    #     if bits[0]:
    #         res.append(f"({self.int_t}) (((uint32_t* ) {base})[0])")
    #     if bits[1]:
    #         res.append(
    #             self._SHL_exp(
    #                 f"({self.int_t}) (((uint16_t* ) {base})[{2*bits[0]}])", 32 * bits[0]
    #             )
    #         )
    #     if bits[2]:
    #         res.append(
    #             self._SHL_exp(
    #                 f"({self.int_t}) (((uint8_t* ) {base})[{2*bits[1] + 4*bits[0]}])",
    #                 32 * bits[0] + 16 * bits[1],
    #             )
    #         )
    #     return " | ".join(res)

    @override
    def unpack_and_encode_key(self) -> None:
        if self.explicitKeyTransform:
            nocheck = self.nocheck
            self.nocheck = True
            super().unpack_and_encode_key()
            self.nocheck = nocheck
            return
        self._function_header(
            "int",
            "unpack_and_encode_key",
            [(f"{self.field_elem_t}*", "res"), (f"const {self.int_t}*", "a")],
        )
        self._startBody(nocheck=True)
        self._declare_var(name="key", typ="const uint8_t *const", val="(uint8_t *) a")
        j: int = 0
        taken: int = 0
        clamp = self.keyClamp
        keybitsize = self.pi if self.keysize * 8 > self.pi else self.keysize * 8
        available_bytes = self.keysize
        pos = 0
        pos_byte = 0
        try:
            numkeylimbs: int = next(
                i for i, x in enumerate(cumsum(self.limbbits)) if x >= keybitsize
            )
            for i, bits in enumerate(self.limbbits[: numkeylimbs + 1]):
                pos_byte = pos // 8
                taken = pos % 8
                available_bytes = self.keysize - pos_byte
                if available_bytes < self.wordbytes:
                    pos_byte = self.keysize - self.wordbytes
                    taken += (self.wordbytes - available_bytes) * 8
                key_limb = f"*((uint{self.wordsize}_t *)(key+{pos_byte}))"

                key_bits_exp = self._SHR_exp(key_limb, taken)

                available_bits = self.wordsize - taken
                if available_bits < bits and available_bytes > self.wordbytes:
                    remaining_bits = bits - available_bits
                    key_limb = f"((uint{self.wordsize}_t) (key[{pos_byte + self.wordsize//8}]))"
                    key_bits_exp = _OR(
                        key_bits_exp, self._SHL_exp(key_limb, available_bits)
                    )

                limb_clamp = clamp & (2**bits - 1)
                clamp = clamp >> bits
                self._ASSIGN(
                    f"res->val[{i}]",
                    _AND(
                        key_bits_exp,
                        hex(limb_clamp),
                    ),
                )
                pos += bits

        except StopIteration:
            pass
        self._endBody(nocheck=True)

    @override
    def unpack_field_elem(self) -> None:
        self._function_header(
            "int",
            "unpack_field_elem",
            [(f"{self.field_elem_t}*", "res"), (f"const {self.int_t}*", "a")],
        )
        self._startBody(nocheck=True)
        self._declare_var(name="elem", typ="const uint8_t *const", val="(uint8_t *) a")
        taken: int = 0
        available_bytes = self.buffsize
        pos = 0
        pos_byte = 0
        for i, bits in enumerate(self.limbbits):
            pos_byte = pos // 8
            taken = pos % 8
            available_bytes = self.buffsize - pos_byte
            if available_bytes < self.wordbytes:
                pos_byte = self.buffsize - self.wordbytes
                taken += (self.wordbytes - available_bytes) * 8
            elem_limb = f"*((uint{self.wordsize}_t *)(elem+{pos_byte}))"

            elem_bits_exp = self._SHR_exp(elem_limb, taken)

            available_bits = self.wordsize - taken
            if available_bits < bits and available_bytes > self.wordbytes:
                elem_limb = (
                    f"((uint{self.wordsize}_t) (elem[{pos_byte + self.wordsize//8}]))"
                )
                elem_bits_exp = _OR(
                    elem_bits_exp, self._SHL_exp(elem_limb, available_bits)
                )

            limb_mask = 2**bits - 1
            self._ASSIGN(
                f"res->val[{i}]",
                _AND(
                    elem_bits_exp,
                    hex(limb_mask),
                ),
            )
            pos += bits
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
            [(f"{self.field_elem_t}*", "res"), (f"const {self.int_t}*", "a")],
        )
        self._startBody(nocheck=True)
        self._declare_var(name="elem", typ="const uint8_t *const", val="(uint8_t *) a")
        j: int = 0
        taken: int = 0
        clamp = sum(
            [
                (mask & (2 ** int(bits) - 1)) * 2 ** int(exp)
                for mask, exp, bits in zip(
                    self.encodingMask, cumsum([0] + self.limbbits[:-1]), self.limbbits
                )
            ]
        )
        blockbitsize = self.pi if self.blocksize * 8 > self.pi else self.blocksize * 8
        available_bytes = self.blocksize
        pos = 0
        pos_byte = 0
        try:
            numBlockLimbs: int = next(
                i for i, x in enumerate(cumsum(self.limbbits)) if x > blockbitsize
            )
            encshift: int = (self.blocksize * 8) - sum(self.limbbits[:numBlockLimbs])
            for i, bits in enumerate(self.limbbits[: numBlockLimbs + 1]):
                encBits = 0
                if self.lowerEncode and not self.lastOnlyEnc:
                    if i == 0:
                        encBits = self.encodingMSB.bit_length()
                        bits -= encBits
                        encshift += self.encodingMSB.bit_length()
                    elif i == self.numlimbs - 1:
                        bits += encBits
                pos_byte = pos // 8
                taken = pos % 8
                available_bytes = self.blocksize - pos_byte
                if available_bytes < self.wordbytes:
                    pos_byte = self.blocksize - self.wordbytes
                    taken += (self.wordbytes - available_bytes) * 8
                block_limb = f"*((uint{self.wordsize}_t *)(elem+{pos_byte}))"

                block_bits_exp = self._SHR_exp(block_limb, taken)

                available_bits = self.wordsize - taken
                if available_bits < bits and available_bytes > self.wordbytes:
                    remaining_bits = bits - available_bits
                    block_limb = f"((uint{self.wordsize}_t) (elem[{pos_byte + self.wordsize//8}]))"
                    block_bits_exp = _OR(
                        block_bits_exp, self._SHL_exp(block_limb, available_bits)
                    )
                limb_mask = clamp & (2**bits - 1)
                clamp = clamp >> bits
                block_bits_exp = self._SHL_exp(
                    _AND(
                        block_bits_exp,
                        hex(limb_mask),
                    ),
                    encBits,
                )
                if self.lowerEncode and not self.lastOnlyEnc and i == 0:
                    block_bits_exp = _OR(block_bits_exp, hex(self.encodingMSB))
                self._ASSIGN(f"res->val[{i}]", block_bits_exp)
                pos += bits

            if not self.lowerEncode and not self.lastOnlyEnc and self.encodingMSB != 0:
                self._OREQ(
                    f"res->val[{self.numlimbs-1}]",
                    self._SHL_exp(f"({self.int_t}){hex(self.encodingMSB)}", encshift),
                )
        except StopIteration:
            pass
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
                (f"const {self.int_t}*", "a"),
                ("size_t", "size"),
            ],
        )
        self._startBody(nocheck=True)
        self._declare_var(f"{self.int_t}", "tmp[BUFFSIZE]", "{0}")
        self._CALL("memcpy", ["tmp", "a", "size"], OFLAG=None)
        if not self.lowerEncode:
            self._IF(f"size < {self.blocksize}")
            self._ASSIGN("((uint8_t*)tmp)[size]", f"{hex(self.encodingMSB)}")
            self._ENDIF()
        self._declare_var(
            name="elem", typ="const uint8_t *const", val="(uint8_t *) tmp"
        )
        j: int = 0
        taken: int = 0
        clamp = sum(
            [
                (mask & (2 ** int(bits) - 1)) * 2 ** int(exp)
                for mask, exp, bits in zip(
                    self.encodingMask, cumsum([0] + self.limbbits[:-1]), self.limbbits
                )
            ]
        )
        blockbitsize = self.pi if self.blocksize * 8 > self.pi else self.blocksize * 8
        available_bytes = self.blocksize
        pos = 0
        pos_byte = 0
        try:
            numBlockLimbs: int = next(
                i for i, x in enumerate(cumsum(self.limbbits)) if x > blockbitsize
            )
            encshift: int = (self.blocksize * 8) - sum(self.limbbits[:numBlockLimbs])
            for i, bits in enumerate(self.limbbits[: numBlockLimbs + 1]):
                encBits = 0
                if self.lowerEncode:
                    if i == 0:
                        encBits = self.encodingMSB.bit_length()
                        bits -= encBits
                        encshift += self.encodingMSB.bit_length()
                    elif i == self.numlimbs - 1:
                        bits += encBits
                pos_byte = pos // 8
                taken = pos % 8
                available_bytes = self.blocksize - pos_byte
                if available_bytes < self.wordbytes:
                    pos_byte = self.blocksize - self.wordbytes
                    taken += (self.wordbytes - available_bytes) * 8
                block_limb = f"*((uint{self.wordsize}_t *)(elem+{pos_byte}))"

                block_bits_exp = self._SHR_exp(block_limb, taken)

                available_bits = self.wordsize - taken
                if available_bits < bits and available_bytes > self.wordbytes:
                    remaining_bits = bits - available_bits
                    block_limb = f"((uint{self.wordsize}_t) (elem[{pos_byte + self.wordsize//8}]))"
                    block_bits_exp = _OR(
                        block_bits_exp, self._SHL_exp(block_limb, available_bits)
                    )
                limb_mask = clamp & (2**bits - 1)
                clamp = clamp >> bits
                block_bits_exp = self._SHL_exp(
                    _AND(
                        block_bits_exp,
                        hex(limb_mask),
                    ),
                    encBits,
                )
                if self.lowerEncode and i == 0:
                    block_bits_exp = _OR(block_bits_exp, hex(self.encodingMSB))
                self._ASSIGN(f"res->val[{i}]", block_bits_exp)
                pos += bits

            if not self.lowerEncode and self.encodingMSB != 0:
                self._IF(f"size == {self.blocksize}")
                self._OREQ(
                    f"res->val[{self.numlimbs-1}]",
                    self._SHL_exp(f"({self.int_t}){hex(self.encodingMSB)}", encshift),
                )
                self._ENDIF()
        except StopIteration:
            pass
        self._endBody(nocheck=True)

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
                (f"const {self.field_elem_t}*", "a"),
                (f"const {self.field_elem_t}*", "b"),
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
                (f"const {self.dfield_elem_t}*", "a"),
                (f"const {self.field_elem_t}*", "b"),
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
                (f"const {self.dfield_elem_t}*", "a"),
                (f"const {self.dfield_elem_t}*", "b"),
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
                (f"const {self.field_elem_t}*", "a"),
                (f"const {self.field_elem_t}*", "b"),
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
            # self._ASSIGN("c", self._SHR_exp("d[0]", self.limbbits[0]))
            self._SHR("c", "d[0]", self.limbbits[0], carrysize)
            self._ASSIGN("res->val[0]", _AND("d[0]", self.limbmask(self.limbbits[0])))
        else:
            self._INC("res->val[0]", "c", out_type=self.int_t)
            # self._ASSIGN("c", self._SHR_exp("res->val[0]", self.limbbits[0]))
            self._SHR("c", "res->val[0]", self.limbbits[0], carrysize)
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
                (f"const {self.field_elem_t}*", "a"),
                (f"const {self.field_elem_t}*", "b"),
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

    # def field_mul_asym_no_carry(self) -> None:
    #     self._function_header(
    #         "int",
    #         "field_mul_asym_no_carry",
    #         [
    #             (f"{self.dfield_elem_t}*", "res"),
    #             (f"const {self.field_elem_t}*", "a"),
    #             (f"const {self.field_elem_t}*", "b"),
    #         ],
    #     )
    #     self._startBody()
    #     self._declare_var(f"{self.long_t}", "acc")
    #     if self.need_double_carry_temp():
    #         self._declare_var(f"{self.long_t}", "t")
    #     else:
    #         self._declare_var(f"{self.int_t}", "t")
    #     num_packed_limbs = ceil(self.blocksize/self.wordsize)
    #     base_delta = self.wordsize*8 - self.limbbits[0]
    #     for k in range(0, self.numlimbs):
    #         self._ASSIGN(f"res->val[{k}]", "0")
    #     for k in range(0, self.numlimbs):
    #         for i in range(0, self.numlimbs):
    #             for j in range(0, self.numlimbs):
    #                 kk: int = sum(self.limbbits[0:i]) + sum(self.limbbits[0:j])
    #                 if kk == sum(self.limbbits[0:k]):
    #                     b_bitsize = 8*self.blocksize - (num_packed_limbs-1)*self.wordsize*8
    #                     self._MUL("acc", f"a->val[{i}]", f"b->val[{j}]")
    #                     res_size = self.limbbits[i] + b_bitsize + base_delta
    #                     if res_size > self.wordsize*2*8 - (self.numlimbs-1):
    #                         ovflw_amount = res_size + (self.numlimbs-1) - self.wordsize*2*8
    #                         self._INC(f"res->val[{k+1}]",
    #                                   self._SHR_exp(
    #                                       "acc", self.wordsize*2*8 -
    #                                   )
    #                                    "acc", out_type=self.long_t)
    #                     else:
    #                         self._SHL("acc", "acc", base_delta, wordsize=self.wordsize*2)
    #                     self._INC(f"res->val[{k}]", "acc", out_type=self.long_t)
    #                 elif (
    #                     len(
    #                         [
    #                             x
    #                             for x in cumsum([0] + self.limbbits)
    #                             if x <= (kk - self.pi)
    #                         ]
    #                     )
    #                     == k + 1
    #                 ):
    #                     self._MUL(
    #                         "t",
    #                         f"b->val[{j}]",
    #                         f"{self._SHL_exp(self.delta, kk-self.pi - sum(self.limbbits[0:k]))}",
    #                         long_return=self.need_double_carry_temp(),
    #                     )
    #                     self._MUL(
    #                         "acc",
    #                         f"a->val[{i}]",
    #                         "t",
    #                     )
    #                     self._INC(f"res->val[{k}]", "acc", out_type=self.long_t)
    #         print(file=self.file)
    #     self._endBody()

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
            # self._ASSIGN("c", self._SHR_exp("a->val[0]", self.limbbits[0]))
            self._SHR("c", "a->val[0]", self.limbbits[0], carrysize)
            self._ASSIGN(
                "res->val[0]", _AND("a->val[0]", self.limbmask(self.limbbits[0]))
            )
        else:
            self._INC("res->val[0]", "c", out_type=self.int_t)
            # self._ASSIGN("c", self._SHR_exp("res->val[0]", self.limbbits[0]))
            self._SHR("c", "res->val[0]", self.limbbits[0], carrysize)
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
        # self._ASSIGN("c", self._SHR_exp("res->val[0]", self.limbbits[0]))
        self._SHR("c", "res->val[0]", self.limbbits[0], carrysize)
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
            [(f"{self.field_elem_t}*", "res"), (f"const {self.field_elem_t}*", "a")],
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
            [(f"{self.field_elem_t}*", "res"), (f"const {self.field_elem_t}*", "a")],
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
                for j in range(i, self.numlimbs):
                    kk: int = sum(self.limbbits[0:i]) + sum(self.limbbits[0:j])
                    if kk == sum(self.limbbits[0:k]):
                        self._MUL("acc", f"a->val[{i}]", f"a->val[{j}]")
                        self._INC(f"d[{k}]", "acc", out_type=self.long_t)
                        if i != j:
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
                            f"a->val[{j}]",
                            f"{self._SHL_exp(self.delta, kk-self.pi - sum(self.limbbits[0:k]))}",
                            long_return=self.need_double_carry_temp(),
                        )
                        self._MUL(
                            "acc",
                            f"a->val[{i}]",
                            "t",
                        )
                        self._INC(f"d[{k}]", "acc", out_type=self.long_t)
                        if i != j:
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
            # self._ASSIGN("c", self._SHR_exp("d[0]", self.limbbits[0]))
            self._SHR("c", "d[0]", self.limbbits[0], carrysize)
            self._ASSIGN("res->val[0]", _AND("d[0]", self.limbmask(self.limbbits[0])))
        else:
            self._INC("res->val[0]", "c", out_type=self.int_t)
            # self._ASSIGN("c", self._SHR_exp("res->val[0]", self.limbbits[0]))
            self._SHR("c", "res->val[0]", self.limbbits[0], carrysize)
            self._ASSIGN(
                "res->val[0]", _AND("res->val[0]", self.limbmask(self.limbbits[0]))
            )
        if self.numlimbs > 1:
            self._INC("res->val[1]", "c", out_type=self.int_t)
        else:
            self._INC("res->val[0]", "c", out_type=self.int_t)
        self._endBody()

    @override
    def square_reduce(self) -> None:
        self._function_header(
            "int",
            "field_sqr_reduce",
            [(f"{self.field_elem_t}*", "res"), (f"const {self.field_elem_t}*", "a")],
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
            [(f"{self.dfield_elem_t}*", "res"), (f"const {self.field_elem_t}*", "a")],
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
                for j in range(i, self.numlimbs):
                    kk: int = sum(self.limbbits[0:i]) + sum(self.limbbits[0:j])
                    if kk == sum(self.limbbits[0:k]):
                        self._MUL("acc", f"a->val[{i}]", f"a->val[{j}]")
                        self._INC(f"res->val[{k}]", "acc", out_type=self.long_t)
                        if i != j:
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
                            f"a->val[{j}]",
                            f"{self._SHL_exp(self.delta, kk-self.pi - sum(self.limbbits[0:k]))}",
                            long_return=self.need_double_carry_temp(),
                        )
                        self._MUL(
                            "acc",
                            f"a->val[{i}]",
                            "t",
                        )
                        self._INC(f"res->val[{k}]", "acc", out_type=self.long_t)
                        if i != j:
                            self._INC(f"res->val[{k}]", "acc", out_type=self.long_t)
            print(file=self.file)
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
    def _endBody(self, nocheck=False, retVal=None) -> None:
        if retVal is not None:
            if nocheck:
                print(" " * self.tabdepth + f"return {retVal};", file=self.file)
            else:
                raise ValueError(
                    "Can only return values for function that are not checked for overflow"
                )
        else:
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
    def footer(self) -> None:
        if not self.nocheck:
            for fun in self.funs:
                print(
                    f"#define {fun}(...) ({{if({fun}(__VA_ARGS__))"
                    + '{printf("Integer overflow in %s:%d\\n", __FILE__, __LINE__);}})',
                    file=self.file,
                )
