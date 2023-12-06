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
from typing import FrozenSet, List, Optional
from typing_extensions import override
from numpy import cumsum
from src.field_arithmetic.CrandallArithmeticGenerator import CrandallArithmeticGenerator


def _LO(inp, wordsize) -> str:
    return f"(uint{wordsize}_t) ({inp})"


def _AND(a, b) -> str:
    return f"({a}) & ({b})"


def _OR(a, b) -> str:
    return f"({a}) | ({b})"


class MersenneArithmeticGenerator(CrandallArithmeticGenerator):
    def __init__(
        self,
        pi,
        limbbits,
        num_limbs,
        wordsize,
        buffsize,
        tabdepth=0,
        encodingMSB: int = 0,
        lowerEncode=False,
        blocksize=16,
        keysize=16,
        lastOnlyEnc: bool = False,
        encodingMask: Optional[List[int]] = None,
        file=sys.stdout,
        explicitEncoding=True,
        nocheck=False,
        doublecarry: bool = False,
        doublecarryover: bool = False,
        doublecarry_temp: bool = False,
    ) -> None:
        super().__init__(
            pi,
            1,
            limbbits,
            num_limbs,
            wordsize,
            buffsize,
            tabdepth,
            encodingMSB,
            lowerEncode,
            blocksize,
            keysize,
            lastOnlyEnc,
            encodingMask,
            file,
            explicitEncoding,
            nocheck,
            doublecarry,
            doublecarryover,
            doublecarry_temp,
        )

    @override
    def define_constants(self) -> None:
        print(f"#define PI {self.pi}", file=self.file)
        print(f"#define DOUBLE_WORDSIZE {2*self.wordsize}", file=self.file)
        print(f"#define LIMBMASK (((({self.int_t})1) << LIMBBITS) - 1)", file=self.file)
        print(
            f"#define LIMBMASK2 (((({self.int_t})1) << LIMBBITS2) - 1)", file=self.file
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
                (f"{self.field_elem_t}*", "a"),
                (f"{self.field_elem_t}*", "b"),
            ],
        )
        self._startBody()
        if doublecarry:
            carrysize: int = self.wordsize * 2
        else:
            carrysize = self.wordsize
        self._declare_var(f"uint{carrysize}_t", "c")
        self._declare_var(f"{self.long_t}", "acc")
        self._declare_var(f"{self.long_t}", f"d[{self.numlimbs}]", "{0}")
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
                            "acc",
                            f"a->val[{i}]",
                            self._SHL_exp(
                                f"b->val[{j}]", kk - self.pi - sum(self.limbbits[0:k])
                            ),
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
        if doublecarryover:
            self._ADD(
                "d[0]", "res->val[0]", f"(uint{carrysize}_t) c", res_type=self.long_t
            )
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
                            "acc",
                            f"a->val[{i}]",
                            self._SHL_exp(
                                f"b->val[{j}]", kk - self.pi - sum(self.limbbits[0:k])
                            ),
                        )
                        self._INC(f"res->val[{k}]", "acc", out_type=self.long_t)
            print(file=self.file)
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
        if doublecarryover:
            self._ADD(
                "a->val[0]",
                "res->val[0]",
                f"(uint{carrysize}_t) c",
                res_type=self.long_t,
            )
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
        if self.numlimbs > 1:
            self._declare_var("uint64_t", "c")
        for i in range(0, self.numlimbs - 1):
            self._SHR("c", f"a->val[{i}]", self.limbbits[i], self.wordsize)
            self._ASSIGN(
                f"res->val[{i}]",
                _AND(
                    _LO(f"a->val[{i}]", self.wordsize), self.limbmask(self.limbbits[i])
                ),
            )
            self._INCLO(f"a->val[{i+1}]", "c", out_type=self.long_t)
        self._ASSIGN(
            f"res->val[{self.numlimbs-1}]",
            _LO(f"a->val[{self.numlimbs-1}]", self.wordsize),
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
        #             self._INC(f"d[{k}]", "acc", out_type=self.long_t)
        #         elif (
        #             len([x for x in cumsum([0] + self.limbbits) if x <= (kk - self.pi)])
        #             == k + 1
        #         ):
        #             if cnt > 1:
        #                 self._MUL(
        #                     "acc",
        #                     f"a->val[{i}]",
        #                     f"a->val[{j}] * "
        #                     + self._SHL_exp(
        #                         f"{cnt}", kk - self.pi - sum(self.limbbits[0:k])
        #                     ),
        #                 )
        #             else:
        #                 self._MUL(
        #                     "acc",
        #                     f"a->val[{i}]",
        #                     self._SHL_exp(
        #                         f"a->val[{j}]", kk - self.pi - sum(self.limbbits[0:k])
        #                     ),
        #                 )
        #             self._INC(f"d[{k}]", "acc", out_type=self.long_t)
        #     print(file=self.file)
        # print(file=self.file)
        # for i in range(0, self.numlimbs - 1):
        #     self._SHR("c", f"d[{i}]", self.limbbits[i], carrysize)
        #     self._ASSIGN(
        #         f"res->val[{i}]",
        #         _AND(_LO(f"d[{i}]", self.wordsize), self.limbmask(self.limbbits[i])),
        #     )
        #     self._INCLO(f"d[{i+1}]", "c", out_type=self.long_t)
        # self._SHR("c", f"d[{self.numlimbs-1}]", self.limbbits[-1], carrysize)
        # self._ASSIGN(
        #     f"res->val[{self.numlimbs-1}]",
        #     _AND(
        #         _LO(f"d[{self.numlimbs-1}]", self.wordsize),
        #         self.limbmask(self.limbbits[-1]),
        #     ),
        # )
        # if doublecarryover:
        #     self._ADD(
        #         "d[0]",
        #         "res->val[0]",
        #         f"(uint{carrysize}_t) c * {self.delta}",
        #         res_type=self.long_t,
        #     )
        #     self._ASSIGN("c", self._SHR_exp("d[0]", self.limbbits[0]))
        #     self._ASSIGN("res->val[0]", _AND("d[0]", self.limbmask(self.limbbits[0])))
        # else:
        #     self._INC("res->val[0]", f"c * {self.delta}", out_type=self.int_t)
        #     self._ASSIGN("c", self._SHR_exp("res->val[0]", self.limbbits[0]))
        #     self._ASSIGN(
        #         "res->val[0]", _AND("res->val[0]", self.limbmask(self.limbbits[0]))
        #     )
        # if self.numlimbs > 1:
        #     self._INC("res->val[1]", "c", out_type=self.int_t)
        # else:
        #     self._INC("res->val[0]", "c", out_type=self.int_t)
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
        #             self._INC(f"res->val[{k}]", "acc", out_type=self.long_t)
        #         elif (
        #             len([x for x in cumsum([0] + self.limbbits) if x <= (kk - self.pi)])
        #             == k + 1
        #         ):
        #             if cnt > 1:
        #                 self._MUL(
        #                     "acc",
        #                     f"a->val[{i}]",
        #                     f"a->val[{j}] * "
        #                     + self._SHL_exp(
        #                         f"{cnt}", kk - self.pi - sum(self.limbbits[0:k])
        #                     ),
        #                 )
        #             else:
        #                 self._MUL(
        #                     "acc",
        #                     f"a->val[{i}]",
        #                     self._SHL_exp(
        #                         f"a->val[{j}]", kk - self.pi - sum(self.limbbits[0:k])
        #                     ),
        #                 )
        #             self._INC(f"res->val[{k}]", "acc", out_type=self.long_t)
        #     print(file=self.file)
        self._endBody()

    @override
    def define_types(self) -> None:
        print("typedef unsigned __int128 uint128_t;", file=self.file)
        print(f"typedef {self.int_t} baseint_t;", file=self.file)
        print(
            f"typedef struct int{self.pi}{1}_single {{{self.int_t} val[{self.numlimbs}];}}"
            + f" {self.field_elem_t};",
            file=self.file,
        )
        print(
            f"typedef struct int{self.pi}{1}_double {{{self.long_t} val[{self.numlimbs}];}}"
            + f" {self.dfield_elem_t};",
            file=self.file,
        )
