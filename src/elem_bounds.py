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

from operator import add
from typing import Optional


def join(a: list[int], b: list[int]) -> list[int]:
    return list(map(max, a, b))


class CrandallFieldElemBounds:
    def __init__(
        self,
        pi: int,
        delta: int,
        lamb: int,
        numlimbs: int,
        wordsize: int,
        lambP: Optional[int] = None,
    ) -> None:
        self.pi: int = pi
        self.delta: int = delta
        self.lamb: int = lamb
        self.lambP: int = lamb
        if lambP:
            self.lambP = lambP
        self.numlimbs: int = numlimbs
        self.wordsize: int = wordsize

    def findFixpoint(
        self, maxits=100
    ) -> tuple[list[int], list[int], list[int], int, int, bool]:
        cbounds: list[int]
        new_bounds: list[int]
        new_dbounds: list[int]
        add_bounds: list[int] = []
        old_add_bounds: list[int] = []
        old_dbounds: list[int] = []
        old_bounds: list[int] = []
        old_cbound: int = 0
        new_dbounds = self.getMulBounds()
        new_bounds, cbounds = self.getCarryBounds(new_dbounds)
        cbound: int = max(cbounds)
        iterations = 0
        fixpoint = True
        while (
            new_bounds != old_bounds
            or new_dbounds != old_dbounds
            or old_cbound != cbound
        ):  # or old_cbounds != cbounds:
            if iterations >= maxits:
                fixpoint = False
                break
            # print('.', end='', flush=True)
            old_dbounds = new_dbounds
            old_bounds = new_bounds
            old_cbound = cbound
            old_add_bounds = add_bounds
            add_bounds = join(self.getAddBounds(new_bounds), new_bounds)
            self.assert_limb_bound(add_bounds)
            bounds, cbounds = self.getCarryBounds(add_bounds)
            self.assert_limb_bound(bounds)
            new_dbounds = self.getMulBounds(in_bound=bounds)
            self.assert_dlimb_bound(new_dbounds)
            new_dbounds = join(new_dbounds, old_dbounds)
            bounds, cbounds = self.getCarryBounds(new_dbounds)
            self.assert_limb_bound(bounds)
            new_bounds = join(join(new_bounds, bounds), old_bounds)
            cbound = max(old_cbound, max(cbounds))
            add_bounds = join(old_add_bounds, add_bounds)
            self.assert_carry_bound(new_bounds, cbound, doubleCarry=True)
            iterations += 1
        return new_dbounds, new_bounds, add_bounds, cbound, iterations, fixpoint

    def check_dlimb_bound(self, limbs) -> bool:
        return all(map(lambda b: b < 2 ** (2 * self.wordsize), limbs))

    def check_limb_bound(self, limbs) -> bool:
        return all(map(lambda b: b < 2 ** (self.wordsize), limbs))

    def check_carry_bound(self, limbs, c, doubleCarry=False) -> bool:
        cSize: int = 2 * self.wordsize if doubleCarry else self.wordsize
        return c < 2 ** cSize and all(map(lambda b: b < 2 ** (self.wordsize), limbs))

    def assert_dlimb_bound(self, limbs) -> None:
        if not self.check_dlimb_bound(limbs):
            raise OverflowError

    def assert_limb_bound(self, limbs) -> None:
        if not self.check_limb_bound(limbs):
            raise OverflowError

    def assert_carry_bound(self, limbs, c, doubleCarry=False) -> None:
        if not self.check_carry_bound(limbs, c, doubleCarry):
            raise OverflowError

    def getAddBounds(
        self, a_bounds: Optional[list[int]] = None, b_bounds: Optional[list[int]] = None
    ) -> list[int]:
        if a_bounds is None:
            a_bounds = [2**self.lamb - 1] * (self.numlimbs - 1) + [
                2**self.lambP - 1
            ]

        if b_bounds is None:
            b_bounds = [2**self.lamb - 1] * (self.numlimbs - 1) + [
                2**self.lambP - 1
            ]

        return list(map(add, a_bounds, b_bounds))

    def getIthMulBound(self, i: int, in_bound: Optional[list[int]] = None) -> int:
        if in_bound is None or len(in_bound) < self.numlimbs:
            if i < self.numlimbs - 1:
                return (2**self.lamb - 1) ** 2 * i + (
                    2 * (2**self.lamb - 1) * (2**self.lambP - 1)
                    + (self.numlimbs - i - 2) * (2**self.lamb - 1) ** 2
                ) * self.delta * 2 ** (self.lamb - self.lambP)
            if i == self.numlimbs - 1:
                return (2**self.lamb - 1) ** 2 * i + (
                    2**self.lambP - 1
                ) ** 2 * self.delta * 2 ** (self.lamb - self.lambP)
            if i == self.numlimbs:
                return (self.numlimbs - 2) * (2**self.lamb - 1) ** 2 + 2 * (
                    2**self.lamb - 1
                ) * (2**self.lambP - 1)
            return -1

        bound = 0
        for j in range(0, i):
            bound += in_bound[j] * in_bound[i - 1 - j]
        for j in range(i, self.numlimbs):
            bound += (
                in_bound[j]
                * in_bound[self.numlimbs - j]
                * self.delta
                * 2 ** (self.lamb - self.lambP)
            )
        return bound

    def getMulBounds(self, in_bound: Optional[list[int]] = None) -> list[int]:
        return list(
            map(lambda x: self.getIthMulBound(x, in_bound), range(1, self.numlimbs + 1))
        )

    def getCarryBounds(self, mul_bounds: list[int]) -> tuple[list[int], list[int]]:
        bounds: list[int] = [0] * self.numlimbs
        cbounds: list[int] = []
        cbound = 0
        for i, ll in zip(
            range(self.numlimbs), [self.lamb] * (self.numlimbs - 1) + [self.lambP]
        ):
            mulbound: int = mul_bounds[i] + cbound
            cbound: int = mulbound >> ll
            cbounds.append(cbound)
            bounds[i] = mulbound % (2**ll)
        bounds[0] += cbound * self.delta
        cbound = bounds[0] >> self.lamb
        cbounds.append(cbound)
        bounds[0] = bounds[0] % (2**self.lamb)
        bounds[1] += cbound
        return bounds, cbounds


if __name__ == "__main__":
    x = CrandallFieldElemBounds(150, 3, 30, 5, 32)
    dlimbs, slimbs, add_bounds, c, its, fp = x.findFixpoint()
    print(f"Iterations: {its}")
    print(f"Fixpoint: {fp}")
    print(f"Carry bounds satisfied: {x.check_carry_bound(slimbs,c, True)}")
    print(f"Limb bounds satisfied: {x.check_dlimb_bound(dlimbs)}")
    print(f"Addition limb bounds: {x.check_dlimb_bound(add_bounds)}")
