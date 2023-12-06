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

from collections import Counter
import sys
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


class PrecomputingCrandallArithmeticGenerator(CrandallArithmeticGenerator):
    def __init__(
        self,
        pi: int,
        delta: int,
        limbbits: List[int],
        num_limbs: int,
        wordsize: int,
        buffsize: int,
        tabdepth: int = 0,
        encodingMSB=0,
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
            pi,
            delta,
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
        self.pfield_elem_t: str = "field_elem_precomputed_t"

    @override
    def fieldmul_funs(
        self, doublecarryover: bool = False, doublecarry: bool = False
    ) -> None:
        super().fieldmul_funs(doublecarryover, doublecarry)
        print(file=self.file)
        self.precompute_factor()
        print(file=self.file)
        self.field_mul_precomputed(doublecarryover, doublecarry)
        print(file=self.file)
        self.field_mul_precomputed_no_carry()
        print(file=self.file)
        self.field_mul_precomputed_reduce()
        print(file=self.file)
        self.square_precomputed(doublecarryover, doublecarry)
        print(file=self.file)
        self.square_precomputed_no_carry()
        print(file=self.file)
        self.square_precomputed_reduce()

    @override
    def unified_api(self) -> None:
        print(
            "#define GET_MACRO(_1,_2,_3,NAME,...) NAME",
            file=self.file,
        )
        print(
            "#define PRECOMPUTED(name) name",
            file=self.file,
        )
        print(
            "#define NOT_PRECOMPUTED(name) name##_not_precomputed",
            file=self.file,
        )
        print(
            "#define DECLARE_PC_ELEM(name)\\\n"
            + "  field_elem_t NOT_PRECOMPUTED(name);\\\n"
            + "  field_elem_precomputed_t PRECOMPUTED(name);",
            file=self.file,
        )
        print(
            "#define DECLARE_PC_ELEM_ARRAY(name, size)\\\n"
            + "  field_elem_t NOT_PRECOMPUTED(name)[size];\\\n"
            + "  field_elem_precomputed_t PRECOMPUTED(name)[size];",
            file=self.file,
        )
        print(
            "#define UNPACK_PC_FIELD_ELEM_SINGLE(name,buff)\\\n"
            + "  unpack_field_elem(&NOT_PRECOMPUTED(name), (baseint_t *) buff);",
            file=self.file,
        )
        print(
            "#define UNPACK_PC_FIELD_ELEM_ARRAY(name,buff,index)\\\n"
            + "  unpack_field_elem(&NOT_PRECOMPUTED(name)[index], (baseint_t *) buff);",
            file=self.file,
        )
        print(
            "#define UNPACK_PC_FIELD_ELEM(...)\\\n"
            + "  GET_MACRO(__VA_ARGS__,UNPACK_PC_FIELD_ELEM_ARRAY,UNPACK_PC_FIELD_ELEM_SINGLE)(__VA_ARGS__)",
            file=self.file,
        )
        print(
            "#define INIT_PC_KEY(dst, src)\\\n" + "  precompute_factor(dst, src);",
            file=self.file,
        )
        print(
            "#define FIELD_MUL_PC(dest,srcA,srcB)\\\n"
            + "  field_mul_precomputed(dest, srcA, srcB);",
            file=self.file,
        )
        print(
            "#define FIELD_MUL_PC_NO_CARRY(dest,srcA,srcB)\\\n"
            + "  field_mul_precomputed_no_carry(dest, srcA, srcB);",
            file=self.file,
        )
        print(
            "#define FIELD_MUL_PC_REDUCE(dest,srcA,srcB)\\\n"
            + "  field_mul_precomputed_reduce(dest, srcA, srcB);",
            file=self.file,
        )
        print(
            "#define FIELD_SQR_PC(dest,srcA,srcB)\\\n"
            + "  field_sqr_precomputed(dest, srcA, srcB);",
            file=self.file,
        )
        print(
            "#define FIELD_SQR_PC_NO_CARRY(dest,srcA,srcB)\\\n"
            + "  field_sqr_precomputed_no_carry(dest, srcA, srcB);",
            file=self.file,
        )
        print(
            "#define FIELD_SQR_PC_REDUCE(dest,srcA,srcB)\\\n"
            + "  field_sqr_precomputed_reduce(dest, srcA, srcB);",
            file=self.file,
        )

    @override
    def define_types(self) -> None:
        super().define_types()
        print(
            f"typedef struct int{self.pi}{self.delta}_single_p {{uint{self.wordsize}_t"
            + f" val[{self.numlimbs}][{self.numlimbs}][{self.numlimbs}];}} {self.pfield_elem_t};",
            file=self.file,
        )

    def precompute_factor(self) -> None:
        self._function_header(
            "int",
            "precompute_factor",
            [(f"{self.pfield_elem_t}*", "res"), (f"{self.field_elem_t}*", "b")],
        )
        self._startBody()
        self._CALL(
            "memcpy",
            ["res->val[0][0]", "b", f"sizeof({self.field_elem_t})"],
            OFLAG=False,
        )
        for k in range(0, self.numlimbs):
            for i in range(0, self.numlimbs):
                for j in range(0, self.numlimbs):
                    kk: int = sum(self.limbbits[0:i]) + sum(self.limbbits[0:j])
                    if kk == sum(self.limbbits[0:k]):
                        self._ASSIGN(f"res->val[{i}][{j}][{k}]", f"b->val[{j}]")
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
                        factor: str = self._SHL_exp(
                            self.delta, kk - self.pi - sum(self.limbbits[0:k])
                        )
                        self._MUL(
                            f"res->val[{i}][{j}][{k}]",
                            f"b->val[{j}]",
                            f"{factor}",
                            long_return=False,
                        )
        self._endBody()

    def field_mul_precomputed(
        self, doublecarryover: bool = False, doublecarry: bool = False
    ) -> None:
        self._function_header(
            "int",
            "field_mul_precomputed",
            [
                (f"{self.field_elem_t}*", "res"),
                (f"{self.field_elem_t}*", "a"),
                (f"{self.pfield_elem_t}*", "b"),
            ],
        )
        self._startBody()
        if doublecarry:
            carrysize: int = self.wordsize * 2
        else:
            carrysize = self.wordsize
        self._declare_var(f"uint{carrysize}_t", "c")
        self._declare_var(f"uint{2*self.wordsize}_t", "acc")
        self._declare_var(f"uint{2*self.wordsize}_t", f"d[{self.numlimbs}]", "{0}")
        for k in range(0, self.numlimbs):
            for i in range(0, self.numlimbs):
                for j in range(0, self.numlimbs):
                    kk: int = sum(self.limbbits[0:i]) + sum(self.limbbits[0:j])
                    if kk == sum(self.limbbits[0:k]):
                        self._MUL("acc", f"a->val[{i}]", f"b->val[{i}][{j}][{k}]")
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
                        self._MUL("acc", f"a->val[{i}]", f"b->val[{i}][{j}][{k}]")
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
                "d[0]",
                "res->val[0]",
                f"(uint{carrysize}_t) c * {self.delta}",
                res_type=self.long_t,
            )
            self._ASSIGN("c", self._SHR_exp("d[0]", self.limbbits[0]))
            self._ASSIGN("res->val[0]", _AND("d[0]", self.limbmask(self.limbbits[0])))
        else:
            self._INC("res->val[0]", f"c * {self.delta}", out_type=self.int_t)
            self._ASSIGN("c", self._SHR_exp("res->val[0]", self.limbbits[0]))
            self._ASSIGN(
                "res->val[0]", _AND("res->val[0]", self.limbmask(self.limbbits[0]))
            )
        if self.numlimbs > 1:
            self._INC("res->val[1]", "c", out_type=self.int_t)
        else:
            self._INC("res->val[0]", "c", out_type=self.int_t)
        self._endBody()

    def field_mul_precomputed_no_carry(self) -> None:
        self._function_header(
            "int",
            "field_mul_precomputed_no_carry",
            [
                (f"{self.dfield_elem_t}*", "res"),
                (f"{self.field_elem_t}*", "a"),
                (f"{self.pfield_elem_t}*", "b"),
            ],
        )
        self._startBody()
        self._declare_var(f"uint{2*self.wordsize}_t", "acc")
        for k in range(0, self.numlimbs):
            self._ASSIGN(f"res->val[{k}]", "0")
            for i in range(0, self.numlimbs):
                for j in range(0, self.numlimbs):
                    kk: int = sum(self.limbbits[0:i]) + sum(self.limbbits[0:j])
                    if kk == sum(self.limbbits[0:k]):
                        self._MUL("acc", f"a->val[{i}]", f"b->val[{i}][{j}][{k}]")
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
                        self._MUL("acc", f"a->val[{i}]", f"b->val[{i}][{j}][{k}]")
                        self._INC(f"res->val[{k}]", "acc", out_type=self.long_t)
            print(file=self.file)
        self._endBody()

    def field_mul_precomputed_reduce(self) -> None:
        self._function_header(
            "int",
            "field_mul_precomputed_reduce",
            [
                (f"{self.field_elem_t}*", "res"),
                (f"{self.field_elem_t}*", "a"),
                (f"{self.pfield_elem_t}*", "b"),
            ],
        )
        self._startBody()
        self._declare_var(self.field_elem_t, "tmp")
        self._CALL("field_mul_precomputed", ["&tmp", "a", "b"], OFLAG=not self.nocheck)
        self._CALL("reduce", ["res", "&tmp"], OFLAG=not self.nocheck)
        self._endBody()

    def square_precomputed(
        self, doublecarryover: bool = False, doublecarry: bool = False
    ) -> None:
        self._function_header(
            "int",
            "field_sqr_precomputed",
            [(f"{self.field_elem_t}*", "res"), (f"{self.pfield_elem_t}*", "a")],
        )
        self._startBody()
        self._CALL(
            "field_mul_precomputed",
            ["res", f"({self.field_elem_t}*) a->val[0][0]", "a"],
            OFLAG=not self.nocheck,
        )
        # FIXME!
        # if doublecarry:
        #     carrysize: int = self.wordsize * 2
        # else:
        #     carrysize = self.wordsize
        # self._declare_var(f"uint{carrysize}_t", "c")
        # self._declare_var(f"uint{2*self.wordsize}_t", "acc")
        # self._declare_var(f"uint{2*self.wordsize}_t", f"d[{self.numlimbs}]", "{0}")
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
        #                 f"a->val[{i}][{j}][{j}]",
        #                 f"a->val[{i}][{j}][{k}]" + (f" * {cnt}" if cnt > 1 else ""),
        #             )
        #             self._INC(f"d[{k}]", "acc")
        #         elif (
        #             len([x for x in cumsum([0] + self.limbbits) if x <= (kk - self.pi)])
        #             == k + 1
        #         ):
        #             self._MUL(
        #                 "acc",
        #                 f"a->val[{i}][{j}][{j}]",
        #                 f"a->val[{i}][{j}][{k}]" + (f" * {cnt}" if cnt > 1 else ""),
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
        # if doublecarryover:
        #     self._ADD("d[0]", "res->val[0]", f"(uint{carrysize}_t) c * {self.delta}")
        #     self._ASSIGN("c", self._SHR_exp("d[0]", self.limbbits[0]))
        #     self._ASSIGN("res->val[0]", _AND("d[0]", self.limbmask(self.limbbits[0])))
        # else:
        #     self._INC("res->val[0]", f"c * {self.delta}")
        #     self._ASSIGN("c", self._SHR_exp("res->val[0]", self.limbbits[0]))
        #     self._ASSIGN(
        #         "res->val[0]", _AND("res->val[0]", self.limbmask(self.limbbits[0]))
        #     )
        # if self.numlimbs > 1:
        #     self._INC("res->val[1]", "c")
        # else:
        #     self._INC("res->val[0]", "c")
        self._endBody()

    def square_precomputed_reduce(self) -> None:
        self._function_header(
            "int",
            "field_sqr_precomputed_reduce",
            [(f"{self.field_elem_t}*", "res"), (f"{self.pfield_elem_t}*", "a")],
        )
        self._startBody()
        self._declare_var(self.field_elem_t, "tmp")
        self._CALL("field_sqr_precomputed", ["&tmp", "a"], OFLAG=not self.nocheck)
        self._CALL("reduce", ["res", "&tmp"], OFLAG=not self.nocheck)
        self._endBody()

    def square_precomputed_no_carry(self) -> None:
        self._function_header(
            "int",
            "field_sqr_precomputed_no_carry",
            [(f"{self.dfield_elem_t}*", "res"), (f"{self.pfield_elem_t}*", "a")],
        )
        self._startBody()
        self._CALL(
            "field_mul_precomputed_no_carry",
            ["res", f"({self.field_elem_t}*) a->val[0][0]", "a"],
            OFLAG=not self.nocheck,
        )
        # FIXME!
        # self._declare_var(f"uint{2*self.wordsize}_t", "acc")
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
        #                 f"a->val[{i}][{j}][{j}]",
        #                 f"a->val[{i}][{j}][{k}]" + (f" * {cnt}" if cnt > 1 else ""),
        #             )
        #             self._INC(f"res->val[{k}]", "acc")
        #         elif (
        #             len([x for x in cumsum([0] + self.limbbits) if x <= (kk - self.pi)])
        #             == k + 1
        #         ):
        #             self._MUL(
        #                 "acc",
        #                 f"a->val[{i}][{j}][{j}]",
        #                 f"a->val[{i}][{j}][{k}]" + (f" * {cnt}" if cnt > 1 else ""),
        #             )
        #             self._INC(f"res->val[{k}]", "acc")
        #     print(file=self.file)
        self._endBody()

    @override
    def footer(self) -> None:
        if not self.nocheck:
            for fun in self.funs:
                print(
                    f"#define {fun}(...) ({{if({fun}(__VA_ARGS__))"
                    + '{printf("Integer overflow in %s:%d\\n", __FILE__, __LINE__);}})',
                    file=self.file,
                )
