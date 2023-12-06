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

import os
from copy import deepcopy
from operator import add
from typing import Optional, Union
from abc import ABC, abstractmethod
from clang import cindex as cx


class CrandallPrimeField:
    def __init__(
        self,
        pi: int,
        delta: int,
        limbs: int,
        wordsize: int,
        ll: int,
        llp: Optional[int] = None,
    ) -> None:
        self.pi: int = pi
        self.delta: int = delta
        self.limbs: int = limbs
        self.wordsize: int = wordsize
        self.ll: int = ll
        self.llp: int = ll
        if llp is not None:
            self.llp = llp

    def __eq__(self, other) -> bool:
        res = True
        res &= self.pi == other.pi
        res &= self.delta == other.delta
        res &= self.limbs == other.limbs
        res &= self.wordsize == other.wordsize
        res &= self.ll == other.ll
        res &= self.llp == other.llp
        return res


class BoundsCheckedVariable(ABC):
    @abstractmethod
    def join(self, other) -> "BoundsCheckedVariable":
        pass

    @abstractmethod
    def check_bounds(self) -> bool:
        pass

    def assert_bounds(self) -> None:
        if not self.check_bounds:
            raise OverflowError


class Integer(BoundsCheckedVariable):
    def __init__(self, bound: int, size: int) -> None:
        self.bound: int = bound
        self.size: int = size

    def __eq__(self, other) -> bool:
        return self.size == other.size and self.bound == other.bound

    def join(self, other) -> "Integer":
        if self.size != other.size:
            raise Exception("Incompatible Arguments")
        return Integer(max(self.bound, other.bound), self.size)

    def check_bounds(self) -> bool:
        return self.bound < 2 ** (self.size)


class FieldElem(BoundsCheckedVariable):
    def __init__(
        self, field: CrandallPrimeField, bound: Optional[list[int]] = None
    ) -> None:
        self.field: CrandallPrimeField = field
        self.bound: list[int]
        if bound is None:
            self.bound: list[int] = [0] * field.limbs
        else:
            self.bound = bound.copy()

    def __eq__(self, other) -> bool:
        return self.field == other.field and self.bound == other.bound

    def join(self: "FieldElem", other: "FieldElem") -> "FieldElem":
        if other.field != self.field:
            raise Exception("Cannot join elements from different fields")
        bound = list(map(max, self.bound, other.bound))
        return FieldElem(self.field, bound=bound)

    def check_bounds(self) -> bool:
        return all(map(lambda b: b < 2 ** (self.field.wordsize), self.bound))


class DFieldElem(BoundsCheckedVariable):
    def __init__(
        self, field: CrandallPrimeField, bound: Optional[list[int]] = None
    ) -> None:
        self.field: CrandallPrimeField = field
        self.bound: list[int]
        if bound is None:
            self.bound: list[int] = [0] * field.limbs
        else:
            self.bound = bound.copy()

    def __eq__(self, other) -> bool:
        return self.field == other.field and self.bound == other.bound

    def join(self: "DFieldElem", other: "DFieldElem") -> "DFieldElem":
        if other.field != self.field:
            raise Exception("Cannot join elements from different fields")
        bound = list(map(max, self.bound, other.bound))
        return DFieldElem(self.field, bound=bound)

    def check_bounds(self) -> bool:
        return all(map(lambda b: b < 2 ** (2 * self.field.wordsize), self.bound))


class State:
    def __init__(
        self,
        field: Optional[CrandallPrimeField] = None,
        variables: Optional[dict[str, BoundsCheckedVariable]] = None,
        doubleCarry: bool = False,
    ):
        if field is None:
            self.field: CrandallPrimeField = CrandallPrimeField(0, 0, 0, 0, 0)
        else:
            self.field = field
        self.doubleCarry: bool = doubleCarry
        if variables is None:
            self.vars: dict[str, BoundsCheckedVariable] = {}
        else:
            self.vars = variables.copy()

    def __eq__(self, other) -> bool:
        res: bool = isinstance(other, State)
        if res:
            res &= self.field == other.field
            res &= self.doubleCarry == other.doubleCarry
            res &= self.vars == other.vars
        return res

    def join(self, other) -> "State":
        if self.field != other.field:
            raise Exception("Incompatible Arguments")
        doubleCarry: bool = self.doubleCarry or other.doubleCarry
        field: CrandallPrimeField = self.field
        variables: dict[str, BoundsCheckedVariable] = self.vars.copy()
        for k, v in other.vars.items():
            if variables.get(k, None) is None:
                variables[k] = v
            else:
                variables[k] = variables[k].join(v)

        return State(field=field, variables=variables, doubleCarry=doubleCarry)


def fieldAdd(
    a: Union[FieldElem, DFieldElem], b: Union[FieldElem, DFieldElem]
) -> Union[FieldElem, DFieldElem]:
    if a.field != b.field:
        raise Exception("Cannot add elements of Different Fields")
    bound = list(map(add, a.bound, b.bound))
    if isinstance(a, DFieldElem) or isinstance(b, DFieldElem):
        return DFieldElem(a.field, bound)
    else:
        return FieldElem(a.field, bound)


def fieldMulCl(a: FieldElem, b: FieldElem) -> DFieldElem:
    if a.field != b.field:
        raise Exception("Cannot multiply elements of Different Fields")
    bound: list[int] = [0] * a.field.limbs
    for i in range(0, a.field.limbs):
        for j in range(0, i + 1):
            bound[i] += a.bound[j] * b.bound[i - j]
        for j in range(i + 1, a.field.limbs):
            bound[i] += (
                a.bound[j]
                * b.bound[a.field.limbs - j - 1]
                * a.field.delta
                * 2 ** (a.field.ll - a.field.llp)
            )
    return DFieldElem(a.field, bound=bound)


def fieldMul(
    a: FieldElem, b: FieldElem, double: bool = False
) -> tuple[FieldElem, Integer, DFieldElem]:
    if a.field != b.field:
        raise Exception("Cannot multiply elements of Different Fields")
    bound: list[int] = [0] * a.field.limbs
    for i in range(0, a.field.limbs):
        for j in range(0, i):
            bound[i] += a.bound[j] * b.bound[i - j]
        for j in range(i, a.field.limbs):
            bound[i] += (
                a.bound[j]
                * b.bound[a.field.limbs - j - 1]
                * a.field.delta
                * 2 ** (a.field.ll - a.field.llp)
            )
    res = DFieldElem(a.field, bound=bound)

    bound: list[int] = [0] * a.field.limbs
    cbounds: list[int] = []
    cbound = 0
    for i, ll in zip(
        range(res.field.limbs), [res.field.ll] * (res.field.limbs - 1) + [res.field.llp]
    ):
        mulbound: int = res.bound[i] + cbound
        cbound: int = mulbound >> ll
        cbounds.append(cbound)
        bound[i] = mulbound % (2**ll)
    bound[0] += cbound * res.field.delta
    cbound = bound[0] >> res.field.ll
    cbounds.append(cbound)
    bound[0] = bound[0] % (2**res.field.ll)
    bound[1] += cbound
    return (
        FieldElem(res.field, bound=bound),
        Integer(max(cbounds), 64 if double else 32),
        res,
    )


def fieldSqrCl(a: FieldElem) -> DFieldElem:
    bound: list[int] = [0] * a.field.limbs
    for i in range(0, a.field.limbs):
        for j in range(0, (i // 2) + 1):
            bound[i] += a.bound[j] * a.bound[i - j] * (2 if i != j else 1)
        for j in range(i + 1, a.field.limbs - (a.field.limbs // 2 - (i + 1) // 2)):
            bound[i] += (
                a.bound[j]
                * a.bound[a.field.limbs - j - 1]
                * a.field.delta
                * 2 ** (a.field.ll - a.field.llp)
                * (2 if i != j else 1)
            )
    return DFieldElem(a.field, bound=bound)


def fielSqr(
    a: FieldElem, double: bool = False
) -> tuple[FieldElem, Integer, DFieldElem]:
    bound: list[int] = [0] * a.field.limbs
    for i in range(0, a.field.limbs):
        for j in range(0, (i // 2) + 1):
            bound[i] += a.bound[j] * a.bound[i - j] * (2 if i != j else 1)
        for j in range(i + 1, a.field.limbs - (a.field.limbs // 2 - (i + 1) // 2)):
            bound[i] += (
                a.bound[j]
                * a.bound[a.field.limbs - j - 1]
                * a.field.delta
                * 2 ** (a.field.ll - a.field.llp)
                * (2 if i != j else 1)
            )
    res = DFieldElem(a.field, bound=bound)

    bound: list[int] = [0] * a.field.limbs
    cbounds: list[int] = []
    cbound = 0
    for i, ll in zip(
        range(res.field.limbs), [res.field.ll] * (res.field.limbs - 1) + [res.field.llp]
    ):
        mulbound: int = res.bound[i] + cbound
        cbound: int = mulbound >> ll
        cbounds.append(cbound)
        bound[i] = mulbound % (2**ll)
    bound[0] += cbound * res.field.delta
    cbound = bound[0] >> res.field.ll
    cbounds.append(cbound)
    bound[0] = bound[0] % (2**res.field.ll)
    bound[1] += cbound
    return (
        FieldElem(res.field, bound=bound),
        Integer(max(cbounds), 64 if double else 32),
        res,
    )


def carry(a: DFieldElem, double: bool = False) -> tuple[FieldElem, Integer, FieldElem]:
    bound: list[int] = [0] * a.field.limbs
    mulbound: list[int] = [0] * a.field.limbs
    cbounds: list[int] = []
    cbound: int = 0
    for i, ll in zip(
        range(a.field.limbs), [a.field.ll] * (a.field.limbs - 1) + [a.field.llp]
    ):
        mulbound[i] = a.bound[i] + cbound
        cbound = mulbound[i] >> ll
        cbounds.append(cbound)
        bound[i] = mulbound[i] % (2**ll)
    bound[0] += cbound * a.field.delta
    cbound = bound[0] >> a.field.ll
    cbounds.append(cbound)
    bound[0] = bound[0] % (2**a.field.ll)
    bound[1] += cbound
    return (
        FieldElem(a.field, bound=bound),
        Integer(max(cbounds), 64 if double else 32),
        FieldElem(a.field, mulbound),
    )


def reduce(a: FieldElem) -> tuple[FieldElem, Integer]:
    bound: list[int] = [0] * a.field.limbs
    mulbound: list[int] = [0] * a.field.limbs
    cbounds: list[int] = []
    cbound: int = a.field.delta
    for i, ll in zip(range(a.field.limbs - 1), [a.field.ll] * (a.field.limbs - 1)):
        mulbound[i] = a.bound[i] + cbound
        cbound = mulbound[i] >> ll
        cbounds.append(cbound)
        bound[i] = mulbound[i] % (2**ll)
    mulbound[a.field.limbs - 1] = a.bound[a.field.limbs - 1] + cbound
    cbound = mulbound[0] >> ll
    cbounds.append(cbound)
    bound[i] = mulbound[i] % (2**ll)


def analyse(poly: str):
    index = cx.Index.create()
    tu = index.parse(os.path.join("src", "polynomial", poly + ".c"))
    poly_def = None
    for i in tu.cursor.get_children():
        if i.kind == cx.CursorKind.FUNCTION_DECL and i.spelling == poly:
            poly_def = i.get_definition()
    if poly_def is None:
        return -1
    body = None
    for i in poly_def.get_children():
        if i.kind == cx.CursorKind.COMPOUND_STMT:
            body = i
            # for j in i.walk_preorder():
            #     print(j.kind, i.spelling, i.displayname, i.result_type.kind, i.result_type.spelling)
    if body is None:
        return -1
    field = CrandallPrimeField(pi=150, delta=5, limbs=5, wordsize=32, ll=30)
    handle_compound(State(field), body)
    print_tree(body)


def handle_decl(st: State, c: cx.Cursor) -> State:
    var_decls: list[cx.Cursor] = []
    res: dict[str, BoundsCheckedVariable] = deepcopy(st.vars)
    if c.kind == cx.CursorKind.DECL_STMT:
        for i in c.get_children():
            if i.kind == cx.CursorKind.VAR_DECL:
                var_decls.append(i)
    for vd in var_decls:
        for v in vd.get_children():
            if v.kind == cx.CursorKind.TYPE_REF:
                if v.displayname == "field_elem_t":
                    res[vd.displayname] = FieldElem(st.field)
                elif v.displayname == "dfield_elem_t":
                    res[vd.displayname] = DFieldElem(st.field)
    return State(st.field, res)


def handle_unpack_call(st: State, c: cx.Cursor) -> State:
    varref: cx.Cursor = list(c.get_children())[1]
    st = deepcopy(st)
    if varref.kind == cx.CursorKind.UNARY_OPERATOR:
        varref = next(varref.get_children())
        if varref.kind == cx.CursorKind.DECL_REF_EXPR:
            variable: BoundsCheckedVariable = st.vars[varref.displayname]
            if isinstance(variable, FieldElem) or isinstance(variable, DFieldElem):
                field: CrandallPrimeField = variable.field
                variable.bound = [2**field.ll] * (field.limbs - 1) + [2**field.llp]
    return st


def handle_add_call(st: State, c: cx.Cursor) -> State:
    st = deepcopy(st)
    varrefs: list[cx.Cursor] = list(c.get_children())[1:]
    if (
        varrefs[0].kind == cx.CursorKind.UNARY_OPERATOR
        and varrefs[1].kind == cx.CursorKind.UNARY_OPERATOR
        and varrefs[2].kind == cx.CursorKind.UNARY_OPERATOR
    ):
        resC: cx.Cursor = next(varrefs[0].get_children())
        leftC: cx.Cursor = next(varrefs[1].get_children())
        rightC: cx.Cursor = next(varrefs[2].get_children())
        if (
            resC.kind == cx.CursorKind.DECL_REF_EXPR
            and leftC.kind == cx.CursorKind.DECL_REF_EXPR
            and rightC.kind == cx.CursorKind.DECL_REF_EXPR
        ):
            res: BoundsCheckedVariable = st.vars[resC.displayname]
            left: BoundsCheckedVariable = st.vars[leftC.displayname]
            right: BoundsCheckedVariable = st.vars[rightC.displayname]
            if (
                (isinstance(res, FieldElem) or isinstance(res, DFieldElem))
                and (isinstance(left, FieldElem) or isinstance(left, DFieldElem))
                and (isinstance(right, FieldElem) or isinstance(right, DFieldElem))
            ):
                st.vars[resC.displayname] = fieldAdd(left, right)
    return st


def handle_add_reduce_call(st: State, c: cx.Cursor) -> State:
    return st


def handle_mul_call(st: State, c: cx.Cursor) -> State:
    st = deepcopy(st)
    varrefs: list[cx.Cursor] = list(c.get_children())[1:]
    if (
        varrefs[0].kind == cx.CursorKind.UNARY_OPERATOR
        and varrefs[1].kind == cx.CursorKind.UNARY_OPERATOR
        and varrefs[2].kind == cx.CursorKind.UNARY_OPERATOR
    ):
        resC: cx.Cursor = next(varrefs[0].get_children())
        leftC: cx.Cursor = next(varrefs[1].get_children())
        rightC: cx.Cursor = next(varrefs[2].get_children())
        if (
            resC.kind == cx.CursorKind.DECL_REF_EXPR
            and leftC.kind == cx.CursorKind.DECL_REF_EXPR
            and rightC.kind == cx.CursorKind.DECL_REF_EXPR
        ):
            res: BoundsCheckedVariable = st.vars[resC.displayname]
            left: BoundsCheckedVariable = st.vars[leftC.displayname]
            right: BoundsCheckedVariable = st.vars[rightC.displayname]
            if (
                isinstance(res, FieldElem)
                and isinstance(left, FieldElem)
                and isinstance(right, FieldElem)
            ):
                resvar, car, mulvar = fieldMul(left, right, st.doubleCarry)
                st.vars[resC.displayname] = resvar
                st.vars["mul:c"] = car
                st.vars["mul:D"] = mulvar
    return st


def handle_mul_no_carry_call(st: State, c: cx.Cursor) -> State:
    st = deepcopy(st)
    varrefs: list[cx.Cursor] = list(c.get_children())[1:]
    if (
        varrefs[0].kind == cx.CursorKind.UNARY_OPERATOR
        and varrefs[1].kind == cx.CursorKind.UNARY_OPERATOR
        and varrefs[2].kind == cx.CursorKind.UNARY_OPERATOR
    ):
        resC: cx.Cursor = next(varrefs[0].get_children())
        leftC: cx.Cursor = next(varrefs[1].get_children())
        rightC: cx.Cursor = next(varrefs[2].get_children())
        if (
            resC.kind == cx.CursorKind.DECL_REF_EXPR
            and leftC.kind == cx.CursorKind.DECL_REF_EXPR
            and rightC.kind == cx.CursorKind.DECL_REF_EXPR
        ):
            res: BoundsCheckedVariable = st.vars[resC.displayname]
            left: BoundsCheckedVariable = st.vars[leftC.displayname]
            right: BoundsCheckedVariable = st.vars[rightC.displayname]
            if (
                isinstance(res, DFieldElem)
                and isinstance(left, FieldElem)
                and isinstance(right, FieldElem)
            ):
                st.vars[resC.displayname] = fieldMulCl(left, right)
    return st


def handle_mul_reduce_call(st: State, c: cx.Cursor) -> State:
    st = deepcopy(st)
    return st


def handle_sqr_call(st: State, c: cx.Cursor) -> State:
    st = deepcopy(st)
    return st


def handle_sqr_no_carry_call(st: State, c: cx.Cursor) -> State:
    st = deepcopy(st)
    return st


def handle_sqr_reduce_call(st: State, c: cx.Cursor) -> State:
    st = deepcopy(st)
    return st


def handle_carry_call(st: State, c: cx.Cursor) -> State:
    st = deepcopy(st)
    varrefs: list[cx.Cursor] = list(c.get_children())[1:]
    if (
        varrefs[0].kind == cx.CursorKind.UNARY_OPERATOR
        and varrefs[1].kind == cx.CursorKind.UNARY_OPERATOR
    ):
        resC: cx.Cursor = next(varrefs[0].get_children())
        argC: cx.Cursor = next(varrefs[1].get_children())
        if (
            resC.kind == cx.CursorKind.DECL_REF_EXPR
            and argC.kind == cx.CursorKind.DECL_REF_EXPR
        ):
            res: BoundsCheckedVariable = st.vars[resC.displayname]
            arg: BoundsCheckedVariable = st.vars[argC.displayname]
            if isinstance(res, FieldElem) and isinstance(arg, DFieldElem):
                resvar, car = carry(arg, st.doubleCarry)
                st.vars[resC.displayname] = resvar
                st.vars["carry_round:c"] = car
    return st


def handle_reduce_call(st: State, c: cx.Cursor) -> State:
    st = deepcopy(st)
    return st


def handle_call(st: State, c: cx.Cursor) -> State:
    if c.kind == cx.CursorKind.CALL_EXPR:
        if c.displayname in [
            "unpack_field_elem",
            "unpack_and_encode_field_elem",
            "unpack_and_encode_last_field_elem",
            "unpack_key",
        ]:
            return handle_unpack_call(st, c)
        elif c.displayname in ["field_add", "field_add_mix", "field_add_dbl"]:
            return handle_add_call(st, c)
        elif c.displayname in ["field_add_reduce"]:
            return handle_add_reduce_call(st, c)
        elif c.displayname in ["field_mul"]:
            return handle_mul_call(st, c)
        elif c.displayname in ["field_mul_no_carry"]:
            return handle_mul_no_carry_call(st, c)
        elif c.displayname in ["field_mul_reduce"]:
            return handle_mul_reduce_call(st, c)
        elif c.displayname in ["field_sqr"]:
            return handle_sqr_call(st, c)
        elif c.displayname in ["field_sqr_no_carry"]:
            return handle_sqr_no_carry_call(st, c)
        elif c.displayname in ["field_sqr_reduce"]:
            return handle_sqr_reduce_call(st, c)
        elif c.displayname in ["carry_round"]:
            return handle_carry_call(st, c)
        elif c.displayname in ["reduce"]:
            return handle_reduce_call(st, c)
    return st


def handle_loop(st: State, c: cx.Cursor) -> State:
    st = deepcopy(st)
    if c.kind == cx.CursorKind.WHILE_STMT:
        pre_state: State = deepcopy(st)
        old_st: State = State()
        body: cx.Cursor = list(c.get_children())[1]
        if body.kind == cx.CursorKind.COMPOUND_STMT:
            while old_st != st:
                old_st = st
                st = handle_compound(st, body).join(old_st)

    return st


def handle_branch(st: State, c: cx.Cursor) -> State:
    st = deepcopy(st)
    if c.kind == cx.CursorKind.IF_STMT:
        children: list[cx.Cursor] = list(c.get_children())
        if_body: cx.Cursor = children[1]
        res_state: State = handle_compound(st, if_body)
        if len(children) > 2:
            else_body: cx.Cursor = children[2]
            res_state = handle_compound(st, else_body).join(res_state)
        st = res_state
    return st


def handle_compound(st: State, c: cx.Cursor) -> State:
    st = deepcopy(st)
    if c.kind == cx.CursorKind.COMPOUND_STMT:
        for child in c.get_children():
            if child.kind == cx.CursorKind.DECL_STMT:
                st = handle_decl(st, child)
            elif child.kind == cx.CursorKind.CALL_EXPR:
                st = handle_call(st, child)
            elif child.kind == cx.CursorKind.WHILE_STMT:
                st = handle_loop(st, child)
            elif child.kind == cx.CursorKind.IF_STMT:
                st = handle_branch(st, child)
            elif child.kind == cx.CursorKind.BINARY_OPERATOR:
                # TODO: Detect and Handle assignments
                print(child.displayname, child.spelling)
                for t in child.get_tokens():
                    print(t.spelling, t.kind)
                pass
            elif child.kind == cx.CursorKind.COMPOUND_STMT:
                st = handle_compound(st, child)
    return st


def print_tree(node: cx.Cursor, prefix: str = "") -> None:
    print(prefix + "|-+", node.kind, node.displayname)
    for i in node.get_children():
        print_tree(i, prefix + "| ")
