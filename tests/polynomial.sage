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

class ClassicalPolynomial:
    def __init__(
        self, tagbytes, keybytes, blockbytes, pi, delta, transform, key_transform
    ):
        self.tagbytes = tagbytes
        self.keybytes = keybytes
        self.blockbytes = blockbytes
        self.pi = pi
        self.delta = delta
        self.transform = transform
        self.key_transform = key_transform
        self.tagbits = 8 * tagbytes
        self.keybits = 8 * keybytes
        self.blockbits = 8 * blockbytes
        self.F = GF(2**pi - delta)
        self.Zp = Integers(2 ** (self.tagbits))
        self.R = PolynomialRing(self.F, "r")

    def eval(self, message, key):
        blocks = [
            message[i : i + self.blockbytes * 2]
            for i in range(0, len(message), self.blockbytes * 2)
        ]
        blocks = self.transform(blocks)
        coeffs = list(map(lambda x: self.F(hex_to_integer(x)), blocks))
        poly = self.R(list(reversed(coeffs)))
        return self.Zp(poly(self.key_transform(key)))


def hex_to_integer(m):
    return int.from_bytes(bytes.fromhex(m), byteorder="little")


def integer_to_hex(n, bytes):
    return int(n).to_bytes(bytes, byteorder="little").hex()
