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
from typing import List, Optional, TextIO, Tuple, Union
from abc import ABC, abstractmethod


class ArithmeticGenerator(ABC):
    def __init__(
        self,
        limbbits: List[int],
        num_limbs: int,
        wordsize: int,
        tabdepth: int = 0,
        encodingMSB: int = 0,
        lowerEncode: bool = False,
        blocksize: int = 16,
        lastOnlyEnc: bool = False,
        encodingMask: Optional[List[int]] = None,
        file=sys.stdout,
        explicitEncoding=True,
    ) -> None:
        self.limbbits: List[int] = limbbits
        self.numlimbs: int = num_limbs
        self.wordsize: int = wordsize
        self.file: TextIO = file
        self.tabdepth: int = tabdepth
        self.blocksize: int = blocksize
        self.encodingMSB: int = encodingMSB
        self.lowerEncode: bool = lowerEncode
        self.lastOnlyEnc: bool = lastOnlyEnc
        self.field_elem_t: str = "field_elem_t"
        self.dfield_elem_t: str = "dfield_elem_t"
        self.int_t: str = f"uint{self.wordsize}_t"
        self.long_t: str = f"uint{2*self.wordsize}_t"
        self.curr_fun: Optional[str] = None
        self.funs = []
        if encodingMask is None:
            self.encodingMask: List[int] = [-1] * num_limbs
        elif len(encodingMask) == 1:
            self.encodingMask: List[int] = encodingMask * num_limbs
        else:
            self.encodingMask = encodingMask
        self.explicitEncoding = explicitEncoding

    def _declare_var(self, typ: str, name: str, val=None) -> None:
        if val is None:
            print(f'{" "*self.tabdepth}{typ} {name};', file=self.file)
        else:
            print(f'{" "*self.tabdepth}{typ} {name} = {val};', file=self.file)

    def _function_header(
        self, typ: str, name: str, args: List[Tuple[str, str]], inline=True
    ) -> None:
        self.curr_fun = name
        self.funs.append(name)
        print(
            f'{" "*self.tabdepth}{"static inline __attribute__((always_inline)) " if inline else ""}'
            + f"{typ} {name}(",
            file=self.file,
        )
        for t, n in args[0:-1]:
            print(f'{" "*(self.tabdepth+4)}{t} {n},', file=self.file)

        print(f'{" "*(self.tabdepth+4)}{args[-1][0]} {args[-1][1]})', file=self.file)

    def _coment(self, comment: str = "") -> None:
        print(f'{" "*(self.tabdepth)}//{comment}', file=self.file)

    def _startBody(self) -> None:
        print(f'{" "*self.tabdepth}{{', file=self.file)
        self.tabdepth += 4

    def _endBody(self) -> None:
        self.tabdepth -= 4
        print(f'{" "*self.tabdepth}}}', file=self.file)
        self.curr_fun = None

    def _SHL_exp(self, a: int | str, shift_amount: int | str) -> str:
        if shift_amount not in ["0", 0]:
            if isinstance(shift_amount, str):
                try:
                    int(shift_amount, base=0)
                except ValueError:
                    shift_amount = f"({shift_amount})"
            if isinstance(a, str):
                try:
                    int(a, base=0)
                except ValueError:
                    a = f"({a})"
            return f"({a} << {shift_amount})"
        return f"({a})"

    def _SHR_exp(self, a: int | str, shift_amount: int | str) -> str:
        if shift_amount not in ["0", 0]:
            if isinstance(shift_amount, str):
                try:
                    int(shift_amount, base=0)
                except ValueError:
                    shift_amount = f"({shift_amount})"
            if isinstance(a, str):
                try:
                    int(a, base=0)
                except ValueError:
                    a = f"({a})"
            return f"({a} >> {shift_amount})"
        return f"({a})"

    def _SHR(self, out, inp, shift, wordsize: Union[int, str]) -> None:
        print(
            f'{" "*self.tabdepth}{out} = (uint{wordsize}_t) {self._SHR_exp(inp, shift)};',
            file=self.file,
        )

    def _SHL(self, out, inp, shift, wordsize: Union[int, str]) -> None:
        print(
            f'{" "*self.tabdepth}{out} = (uint{wordsize}_t) {self._SHL_exp(inp, shift)};',
            file=self.file,
        )

    def _ASSIGN(self, out, inp) -> None:
        print(f'{" "*self.tabdepth}{out} = {inp};', file=self.file)

    def _OREQ(self, out, inp) -> None:
        print(f'{" "*self.tabdepth}{out} |= {inp};', file=self.file)

    def _ANDEQ(self, out, inp) -> None:
        print(f'{" "*self.tabdepth}{out} &= {inp};', file=self.file)

    def _CALL(self, f: str, args: List[str]) -> None:
        print(f'{" "*self.tabdepth}{f}({", ".join(args)});', file=self.file)

    def _IF(self, c) -> None:
        print(f'{" "*self.tabdepth}if ({c}) {{', file=self.file)
        self.tabdepth += 4

    def _ELSE(self) -> None:
        print(f'{" "*(self.tabdepth-4)}}} else {{', file=self.file)

    def _ENDIF(self) -> None:
        self.tabdepth -= 4
        print(f'{" "*self.tabdepth}}}', file=self.file)

    @abstractmethod
    def define_constants(self) -> None:
        pass

    @abstractmethod
    def define_types(self) -> None:
        pass

    @abstractmethod
    def field_mul(
        self, doublecarryover: bool = False, doublecarry: bool = False
    ) -> None:
        pass

    @abstractmethod
    def field_mul_reduce(self) -> None:
        pass

    @abstractmethod
    def field_mul_no_carry(self) -> None:
        pass

    @abstractmethod
    def square(self, doublecarryover: bool = False, doublecarry: bool = False) -> None:
        pass

    @abstractmethod
    def square_reduce(self) -> None:
        pass

    @abstractmethod
    def square_no_carry(self) -> None:
        pass

    @abstractmethod
    def field_addition(self) -> None:
        pass

    @abstractmethod
    def field_addition_mixed(self) -> None:
        pass

    @abstractmethod
    def field_addition_double(self) -> None:
        pass

    @abstractmethod
    def field_addition_reduce(self) -> None:
        pass

    @abstractmethod
    def carry_round(
        self, doublecarryover: bool = False, doublecarry: bool = False
    ) -> None:
        pass

    @abstractmethod
    def reduce(self) -> None:
        pass

    @abstractmethod
    def pack_field_elem(self) -> None:
        pass

    @abstractmethod
    def unpack_key(self) -> None:
        pass

    @abstractmethod
    def unpack_field_elem(self) -> None:
        pass

    def unpack_and_encode_field_elem(self) -> None:
        self._function_header(
            "int",
            "unpack_and_encode_field_elem",
            [(f"{self.field_elem_t}*", "res"), (f"{self.int_t}*", "a")],
        )
        self._startBody()
        self._declare_var("uint8_t", "buff[BUFFSIZE]", "{0}")
        self._CALL("transform_msg", ["buff", "BUFFSIZE", "(uint8_t*) a", "BLOCKSIZE"])
        self._CALL("unpack_field_elem", ["res", "(baseint_t *) buff"])
        self._endBody()

    def unpack_and_encode_last_field_elem(self) -> None:
        self._function_header(
            "int",
            "unpack_and_encode_last_field_elem",
            [
                (f"{self.field_elem_t}*", "res"),
                (f"{self.int_t}*", "a"),
                ("size_t", "a_size"),
            ],
        )
        self._startBody()
        self._declare_var("uint8_t", "buff[BUFFSIZE]", "{0}")
        self._CALL("transform_msg", ["buff", "BUFFSIZE", "(uint8_t*) a", "a_size"])
        self._CALL("unpack_field_elem", ["res", "(baseint_t *) buff"])
        self._endBody()

    def includes(self) -> None:
        print("#include <inttypes.h>", file=self.file)
        print("#include <stddef.h>", file=self.file)
        print("#include <string.h>", file=self.file)
        print('#include "../transform/transform.h"', file=self.file)

    def need_doublecarry(self) -> Tuple[bool, bool]:
        return False, False

    def need_doublecarryover(self) -> bool:
        return False

    def fieldmul_funs(
        self, doublecarryover: bool = False, doublecarry: bool = False
    ) -> None:
        print(file=self.file)
        self.carry_round(doublecarryover, doublecarry)
        print(file=self.file)
        self.reduce()
        print(file=self.file)
        self.field_mul(doublecarryover, doublecarry)
        print(file=self.file)
        self.field_mul_no_carry()
        print(file=self.file)
        self.field_mul_reduce()
        print(file=self.file)
        self.square(doublecarryover, doublecarry)
        print(file=self.file)
        self.square_no_carry()
        print(file=self.file)
        self.square_reduce()
        print(file=self.file)
        self.field_addition()
        print(file=self.file)
        self.field_addition_reduce()
        print(file=self.file)
        self.field_addition_mixed()
        print(file=self.file)
        self.field_addition_double()
        print(file=self.file)
        self.pack_field_elem()
        print(file=self.file)
        self.unpack_field_elem()
        print(file=self.file)
        self.unpack_key()
        print(file=self.file)
        self.unpack_and_encode_field_elem()
        print(file=self.file)
        self.unpack_and_encode_last_field_elem()

    def footer(self) -> None:
        return

    def print_fieldmul(self) -> None:
        doublecarry, doublecarryover = self.need_doublecarry()
        if doublecarryover:
            print("using doublercarryover")
        if doublecarry:
            print("using doublecarry")
        print("#ifndef field_arithmetic_H_", file=self.file)
        print("#define field_arithmetic_H_", file=self.file)
        self.includes()
        self.define_constants()
        self.define_types()
        self.fieldmul_funs(doublecarryover, doublecarry)
        self.footer()
        print("#endif", file=self.file)
        print(file=self.file)
