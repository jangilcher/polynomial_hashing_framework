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


from abc import abstractmethod
from typing import Callable, TypeVar

T = TypeVar("T")
K = TypeVar("K")
Self = TypeVar("Self")
MSG = TypeVar("M")


class BinaryField:
    def __init__(self, fieldsize: int, polynomial: list[int]):
        self.fieldsize: int = fieldsize
        self.polynomial: list[int] = polynomial
        R.<x> = PolynomialRing(GF(2), "x")
        p = sum(map(lambda y: x ^ y, polynomial))
        F.<X> = GF(2 ^ fieldsize, name=x, modulus=p)
        self.F = F
        self.X = X

    def parse_key(self, key, keybits, key_transform):
        if isinstance(key, str):
            k = hex_to_integer(key)
        else:
            k = key
        if self.fieldsize < keybits:
            k = k & ((1 << self.fieldsize) - 1)
        return self.F.from_integer(key_transform(k))

    def parse_field_elems(self, blocks) -> list:
        return list(map(lambda x: self.F.from_integer(hex_to_integer(x)), blocks))

    def tag_as_int(self, tag, tagbits) -> int:
        return tag.to_integer()


class PrimeField:
    def __init__(self, pi, delta):
        self.pi = pi
        self.delta = delta
        self.F = GF(2**pi - delta)

    def parse_key(self, key, keybits, key_transform):
        if isinstance(key, str):
            k = hex_to_integer(key)
        else:
            k = key
        if self.pi < keybits:
            k = k & ((1 << self.pi) - 1)
        return self.F(key_transform(k))

    def parse_field_elems(self, blocks) -> list:
        return list(map(lambda x: self.F(hex_to_integer(x)), blocks))

    def tag_as_int(self, tag, tagbits) -> int:
        return int(Integers(2 ** (tagbits))(tag))


class TestPolynomial:
    def __init__(
        self, field, tagbytes, keybytes, blockbytes, transform, key_transform, hash_transform=None
    ):
        self.field: PrimeField | BinaryField = field
        self.tagbytes = tagbytes
        self.keybytes = keybytes
        self.blockbytes = blockbytes
        self.transform = transform
        self.key_transform = key_transform
        if hash_transform is not None:
            self.hash_transform: Callable[[Self,T,K,MSG], T] = self.hash_transform_table[hash_transform]
        else:
            self.hash_transform = self.identity
        self.tagbits = 8 * tagbytes
        self.keybits = 8 * keybytes
        self.blockbits = 8 * blockbytes
        self.R = PolynomialRing(self.field.F, "r")

    def additive_length_encoding(self, tag: T, keys: K, message: MSG) -> T:
        l = int(ceil(len(message) / 2))
        l_bytes = l.to_bytes(length=self.blockbytes, byteorder="little").hex()
        l_elem = self.field.parse_field_elems([l_bytes])
        res = tag * keys[-2] + l_elem[0] * keys[-1]
        return res

    def multiplicative_length_encoding(self, tag: T, keys: K, message: MSG) -> T:
        l = int(ceil(len(message) / 2))
        l_bytes = l.to_bytes(length=self.blockbytes, byteorder="little").hex()
        l_elem = self.field.parse_field_elems([l_bytes])
        res = (tag + keys[-2]) * (l_elem[0] + keys[-1])
        return res

    def key_reuse_length_encoding(self, tag: T, keys: K, message: MSG) -> T:
        l = int(ceil(len(message) / 2))
        l_bytes = l.to_bytes(length=self.blockbytes, byteorder="little").hex()
        l_elem = self.field.parse_field_elems([l_bytes])
        res = (tag + keys[-1]) * (l_elem[0] + keys[0])
        return res

    def simple_key_reuse_length_encoding(self, tag: T, keys: K, message: MSG) -> T:
        l = int(ceil(len(message) / 2))
        l_bytes = l.to_bytes(length=self.blockbytes, byteorder="little").hex()
        l_elem = self.field.parse_field_elems([l_bytes])
        res = ((tag * keys[0]) + l_elem[0]) * keys[0]
        return res

    def identity(self, sself, tag: T, keys: K, message: MSG) -> T:
        return tag

    hash_transform_table:dict[str, Callable[[Self,T,K,MSG], T]] = {
        "additive_length_encoding": additive_length_encoding,
        "multiplicative_length_encoding": multiplicative_length_encoding,
        "key_reuse_length_encoding": key_reuse_length_encoding,
        "simple_key_reuse_length_encoding": simple_key_reuse_length_encoding,
    }

    def get_message_blocks(self, message) -> list:
        blocks = [
            message[i: i + self.blockbytes * 2]
            for i in range(0, len(message), self.blockbytes * 2)
        ]
        return self.field.parse_field_elems(self.transform(blocks))

    def get_key_blocks(self, key) -> list:
        return [
            self.field.parse_key(
                key[i: i + self.keybytes * 2], self.keybits, self.key_transform)
            for i in range(0, len(key), self.keybytes * 2)
        ]

    def eval(self, message, key) -> int:
        msg_blocks = self.get_message_blocks(message)
        key_blocks = self.get_key_blocks(key)
        tag = self.eval_elems(msg_blocks, key_blocks)

        tag = self.hash_transform(self, tag, key_blocks, message)
        return self.field.tag_as_int(tag, self.tagbits)

    @abstractmethod
    def eval_elems(self, M, r, outerSB=1):
        pass


class classical_polynomial(TestPolynomial):
    def eval_elems(self, M, r, outerSB=1):
        poly = self.R(list(reversed(M)))
        return poly(r[0])


class MMH(TestPolynomial):
    def eval_elems(self, M, r, outerSB=1):
        if len(M) == 0:
            return self.field.F.zero()
        if len(r) == 1 and len(M) > 2:
            r = [r[0]**i for i in range(1,len(M))]
        coeffs = vector(self.field.F, M[:-1])
        tag = coeffs.dot_product(vector(self.field.F, r[:len(coeffs)])) + M[-1]
        return tag


class NMH(TestPolynomial):
    def eval_elems(self, M, r, outerSB=1) -> int:
        if len(r) == 1 and len(M) > 1 and outerSB > 1:
            powers = [r[0]**i for i in range(1,outerSB+1)]
            r = [None] * outerSB
            r[::2] = powers[:outerSB//2]
            r[1::2] = reversed(powers[outerSB//2:])
        if len(M) % 2 == 0:
            even_coeffs = vector(self.field.F, M[::2])
            even_key_blocks = vector(self.field.F, r[:len(M):2])
            uneven_coeffs = vector(self.field.F, M[1::2])
            uneven_key_blocks = vector(self.field.F, r[1:len(M):2])
            tag = (
                even_coeffs+even_key_blocks).dot_product(uneven_coeffs+uneven_key_blocks)
        else:
            even_coeffs = vector(self.field.F, M[:-1:2])
            even_key_blocks = vector(self.field.F, r[:len(M)-1:2])
            uneven_coeffs = vector(self.field.F, M[1:-1:2])
            uneven_key_blocks = vector(self.field.F, r[1:len(M)-1:2])
            tag = (even_coeffs+even_key_blocks).dot_product(uneven_coeffs +
                                                            uneven_key_blocks) + M[-1]
        return tag


class SQH(TestPolynomial):
    def eval_elems(self, M, r, outerSB=1) -> int:
        if len(M) == 0:
            return self.field.F.zero()
        if len(r) == 1 and len(M) > 2:
            r = [r[0]**i for i in range(1,len(M))]
        coeffs = vector(self.field.F, M[:-1])
        key_blocks = vector(self.field.F, r[:len(coeffs)])
        tag = (coeffs+key_blocks).dot_product(coeffs+key_blocks) + M[-1]
        return tag


class uSQH(TestPolynomial):
    def eval_elems(self, M, r, outerSB=1) -> int:
        r = r[0]

        def usqh(m):
            if len(m) == 0:
                return self.field.F.zero()
            elif len(m) == 1:
                return m[0]
            elif len(m) == 2:
                return (m[0] + r) ^ 2 + m[1]
            elif len(m) == 3:
                i = 2 ^ floor(log(len(m)-1, 2).n())
                return usqh(m[:i]) ^ 2 + usqh(m[i:])
        tag = usqh(M)
        return tag


class DCHM(TestPolynomial):
    def eval_elems(self, M, r, outerSB=1) -> int:
        r = r[0]
        if len(M) % 2 == 0:
            key_blocks = [r ^ i for i in range(1, len(M)+1)]
            even_coeffs = vector(self.field.F, M[::2])
            even_key_blocks = vector(self.field.F, key_blocks[:len(M):2])
            uneven_coeffs = vector(self.field.F, M[1::2])
            uneven_key_blocks = vector(self.field.F, key_blocks[1:len(M):2])
            tag = (
                even_coeffs+even_key_blocks).dot_product(uneven_coeffs+uneven_key_blocks)
        else:
            key_blocks = [r ^ i for i in range(1, len(M))]
            even_coeffs = vector(self.field.F, M[:-1:2])
            even_key_blocks = vector(self.field.F, key_blocks[:len(M)-1:2])
            uneven_coeffs = vector(self.field.F, M[1:-1:2])
            uneven_key_blocks = vector(self.field.F, key_blocks[1:len(M)-1:2])
            tag = (even_coeffs+even_key_blocks).dot_product(uneven_coeffs +
                                                            uneven_key_blocks) + M[-1]
        return tag


class BRW(TestPolynomial):
    def eval_elems(self, M, r, outerSB=1) -> int:
        r = r[0]

        def brw(m):
            if len(m) == 0:
                return self.field.F.zero()
            elif len(m) == 1:
                return m[0]
            elif len(m) == 2:
                return m[0] * r + m[1]
            elif len(m) == 3:
                return (r + m[0]) * (r ^ 2 + m[1]) + m[2]
            else:
                i = 2 ^ floor(log(len(m), 2).n())
                return brw(m[:i-1]) * (r ^ i + m[i-1]) + brw(m[i:])
        tag = brw(M)
        return tag


class HKM(TestPolynomial):
    def eval_elems(self, M, r, outerSB=1) -> int:
        if len(r) == 1 and  len(M)//2 + 1 > 1:
            r = [r[0]**i for i in range(1,len(M)//2 + 2)]
        def hkm(m, r):
            if len(m) == 0:
                return self.field.F.zero()
            if len(m) == 1:
                return r[0] + m[0]
            if len(m) == 2:
                return (r[0] + m[0]) * (r[1] + m[1])
            p = len(m)//2
            if len(m) % 2 == 1:
                return m[2*p] + (r[p] + m[2*p-1]) * hkm(m[:2*p-1], r[:p])
            else:
                return (r[p] + m[2*p-1]) * hkm(m[:2*p-1], r[:p])
        tag = hkm(M, r)
        return tag


class TestTreePolynomial(TestPolynomial):
    def __init__(
        self,
        inner_polynomial: TestPolynomial,
        superblocksize: int,
        superkeysize: int,
        *args, **kwargs
    ):
        super().__init__(*args, **kwargs)
        self.inner: TestPolynomial = inner_polynomial
        self.superblocksize: int = superblocksize
        self.superkeysize: int = superkeysize

class tHorner(TestTreePolynomial):
    def eval_elems(self, M, r, outerSB=1) -> int:
        B: int = self.superblocksize
        b: int = self.superkeysize

        def thorner(m, r):
            n = len(m)
            if n == 0:
                return self.field.F.zero()
            if n <= B:
                return self.inner.eval_elems(m, r[:b], outerSB=self.superblocksize)
            else:
                f = floor((len(m) - 1)/B)
                i = B * f
                return thorner(m[:i], r) * r[0]^B + thorner(m[i:], r)
        return thorner(M, r)

class tMMH(TestTreePolynomial):
    def eval_elems(self, M, r, outerSB=1) -> int:
        B: int = self.superblocksize
        b: int = self.superkeysize

        def tmmh(m, r):
            n = len(m)
            if n == 0:
                return self.field.F.zero()
            if n <= B:
                return self.inner.eval_elems(m, r[:b], outerSB=self.superblocksize)
            else:
                f = floor(log(len(m) - 1, 2) - log(B, 2))
                i = B * 2 ^ f
                v = b + f
                return tmmh(m[:i], r[:v]) * r[v] + tmmh(m[i:], r[:v])
        return tmmh(M, r)


class tSQH(TestTreePolynomial):
    def eval_elems(self, M, r, outerSB=1) -> int:
        B: int = self.superblocksize
        b: int = self.superkeysize

        def tsqh(m, r):
            n = len(m)
            if n == 0:
                return self.field.F.zero()
            if n <= B:
                return self.inner.eval_elems(m, r[:b], outerSB=self.superblocksize)
            else:
                f = floor(log(len(m) - 1, 2) - log(B, 2))
                i = B * 2 ^ f
                v = b + f
                return (tsqh(m[:i], r[:v]) + r[v]) ^ 2 + tsqh(m[i:], r[:v])
        return tsqh(M, r)


class tNMH(TestTreePolynomial):
    def eval_elems(self, M, r, outerSB=1) -> int:
        B: int = self.superblocksize
        b: int = self.superkeysize

        def tnmh(m, r):
            n = len(m)
            if n == 0:
                return self.field.F.zero()
            if n <= B:
                return self.inner.eval_elems(m, r[:b], outerSB=self.superblocksize)
            else:
                f = floor(log(len(m) - 1, 3) - log(B, 3))
                i = B * 3 ^ f
                v = b + 2 * f
                return (tnmh(m[:i], r[:v]) + r[v]) * (tnmh(m[i:2*i], r[:v]) + r[v+1]) + tnmh(m[2*i:], r[:v])
        return tnmh(M, r)


class tHKM(TestTreePolynomial):
    def eval_elems(self, M, r, outerSB=1) -> int:
        B: int = self.superblocksize
        b: int = self.superkeysize

        def thkm(m, r):
            n = len(m)
            if n == 0:
                return self.field.F.zero()
            if n <= B:
                return self.inner.eval_elems(m, r[:b], outerSB=self.superblocksize)
            else:
                f = floor(log(len(m) - 1, 3) - log(B, 3))
                i = B * 3 ^ f
                v = b + f
                return thkm(m[:i], r[:v]) * (thkm(m[i:2*i], r[:v]) + r[v]) + thkm(m[2*i:], r[:v])
        return thkm(M, r)


class tBRW(TestTreePolynomial):
    def eval_elems(self, M, r, outerSB=1) -> int:
        B: int = self.superblocksize
        b: int = self.superkeysize

        def tbrw(m, r):
            n = len(m)
            if n == 0:
                return self.field.F.zero()
            if n <= B:
                return self.inner.eval_elems(m, r[:b])
            else:
                f = floor(log(len(m), 2) - log(B+1, 2), outerSB=self.superblocksize)
                i = (B+1) * 2 ^ f
                v = b + f
                return tbrw(m[:i-1], r[:v]) * (r[v] + m[i-1]) + tbrw(m[i:], r[:v])
        return tbrw(M, r)

class MHP(TestTreePolynomial):
    def eval_elems(self, M, r, outerSB=1) -> int:
        B: int = self.superblocksize
        b: int = self.superkeysize
        if b == 0:
            b = 1
            r = [r[0]] + r
        def mhp(m, r):
            n = len(m)
            if n == 0:
                return self.field.F.zero()
            if n <= B:
                return self.inner.eval_elems(m, r[:b], outerSB=self.superblocksize)
            else:
                v = len(r)
                l = ceil(n/B)
                x = 0
                i = 0
                while x < l:
                    idx = x
                    x = binomial(v-b+i-1, v-b)
                    i += 1
                idx *= B
            return r[-1] * mhp(m[:idx], r) + mhp(m[idx:], r[:-1])
        return mhp(M, r)

class d2LHP(TestTreePolynomial):
    def eval_elems(self, M, r, outerSB=1) -> int:
        B: int = self.superblocksize
        b: int = self.superkeysize
        n = len(M)
        if b == 0:
            b = 1
            r = [r[0]] + r
        if len(M) == 0:
            return self.field.F.zero()
        ms = [M[i:i+self.superblocksize] for i in range(0, len(M), self.superblocksize)]
        if len(ms) == 1:
            # print(ms)
            pass
        if len(ms[-1]) < self.superblocksize:
            lastBlocks = ms[-1]
            ms = ms[:-1]
        else:
            lastBlocks = []
        blocks = list(map(lambda x: self.inner.eval_elems(x, r, outerSB=self.superblocksize), ms))
        poly = self.R(list(reversed(blocks)))
        deg = 2**(floor(log(B,2))+1) - 1
        V =  poly(r[0]**(deg+1))
        lastBlocks = [V] + lastBlocks
        poly = self.R(list(reversed(lastBlocks)))
        return poly(r[0])
        def mhp(m, r):
            n = len(m)
            if n == 0:
                return self.field.F.zero()
            if n <= B:
                return self.inner.eval_elems(m, r[:b], outerSB=self.superblocksize)
            else:
                v = len(r)
                l = ceil(n/B)
                x = 0
                i = 0
                while x < l:
                    idx = x
                    x = binomial(v-b+i-1, v-b)
                    i += 1
                idx *= B
            return r[-1] * mhp(m[:idx], r) + mhp(m[idx:], r[:-1])
        nn = n - (n%B)
        acc = mhp(M[:nn], r)
        for m in M[nn:]:
            acc *= r[0]
            acc += m
        return acc

class v1NMH_HORNER(TestTreePolynomial):
    def eval_elems(self, M, r, outerSB=1) -> int:
        B: int = self.superblocksize
        r = r[0]
        def v1NMH(m):
            if len(m) % 2 == 0:
                key_blocks = [r ^ i for i in range(1, len(m)+1)]
                even_coeffs = vector(self.field.F, m[::2])
                even_key_blocks = vector(self.field.F, key_blocks[:len(m):2])
                uneven_coeffs = vector(self.field.F, m[1::2])
                uneven_key_blocks = vector(self.field.F, key_blocks[1:len(m):2])
                tag = (even_coeffs+even_key_blocks).dot_product(uneven_coeffs+uneven_key_blocks)
            else:
                key_blocks = [r ^ i for i in range(1, len(m))]
                even_coeffs = vector(self.field.F, m[:-1:2])
                even_key_blocks = vector(self.field.F, key_blocks[:len(m)-1:2])
                uneven_coeffs = vector(self.field.F, m[1:-1:2])
                uneven_key_blocks = vector(self.field.F, key_blocks[1:len(m)-1:2])
                tag = (even_coeffs+even_key_blocks).dot_product(uneven_coeffs + uneven_key_blocks) + m[-1]
            return tag

        def two_level(m):
            n = len(m)
            if n == 0:
                return self.field.F.zero()
            if n <= B:
                return v1NMH(m)
            else:
                k = r ^ B
                idx = ceil(n/B) - 1
                idx *= B
                return two_level(m[:idx]) * k + two_level(m[idx:])
        return two_level(M)

def hex_to_integer(m):
    return int.from_bytes(bytes.fromhex(m), byteorder="little")


def integer_to_hex(n, bytes):
    return int(n).to_bytes(bytes, byteorder="little").hex()
