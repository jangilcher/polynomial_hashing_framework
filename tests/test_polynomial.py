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

import unittest
import pathlib
import ctypes
import random
import importlib.util
from tests.transform import MessageTransform, KeyTransform, identity

try:
    import sage

    spec = importlib.util.spec_from_file_location(
        name="polynomial", location="tests/polynomial.sage.py"
    )
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
except ModuleNotFoundError:
    pass


class ClassicalPolynomial(unittest.TestCase):
    def __init__(
        self,
        name,
        binname="",
        primetype="0",
        pi=130,
        delta=5,
        blocksize=16,
        keysize=16,
        tagsize=16,
        transform: MessageTransform = identity(),
        key_transform: KeyTransform = identity(),
        numtests=1000,
        full_logs=False,
    ) -> None:
        super().__init__(name)
        self.pi: int = pi
        self.delta: int = delta
        self.blocksize: int = blocksize
        self.keysize: int = keysize
        self.tagsize: int = tagsize
        self.libname: pathlib.Path = pathlib.Path().absolute() / "bin" / f"{binname}.so"
        self.transform: MessageTransform = transform
        self.key_transform: KeyTransform = key_transform
        self.numtests: int = numtests
        self.full_logs: bool = full_logs

    def setUp(self):
        self.lib = ctypes.CDLL(self.libname)

    def test_classical_polynomial(self):
        hash = mod.ClassicalPolynomial(
            tagbytes=self.tagsize,
            keybytes=self.keysize,
            blockbytes=self.blocksize,
            pi=self.pi,
            delta=self.delta,
            transform=self.transform,
            key_transform=self.key_transform,
        )
        for messagesize in range(0, self.blocksize * 16 + 1):
            for it in range(self.numtests):
                with self.subTest(messagesize=messagesize, iteration=it):
                    if messagesize == 0:
                        message = bytes()
                    else:
                        message = random.randrange(0, 2 ** (8 * messagesize)).to_bytes(
                            messagesize, byteorder="little"
                        )
                    key = random.randrange(0, 2 ** (8 * self.keysize))
                    res = hash.eval(message.hex(), key)
                    key_bytes = key.to_bytes(self.keysize, byteorder="little")
                    tag_l = [0] * self.tagsize
                    tag = (ctypes.c_uint8 * len(tag_l))(*tag_l)
                    self.lib.hash(
                        ctypes.pointer(tag),
                        (ctypes.c_uint8 * len(message))(*message),
                        len(message),
                        (ctypes.c_uint8 * len(key_bytes))(*key_bytes),
                    )
                    pretty_message = (
                        message.hex()[: (self.keysize // 2 - 1) * 2]
                        + ".." * (2 + self.keysize % 2)
                        + message.hex()[-(self.keysize // 2 - 1) * 2 :]
                        if messagesize > self.keysize and not self.full_logs
                        else message.hex()
                    )
                    self.assertEqual(
                        bytes(tag).hex(),
                        int(res).to_bytes(self.tagsize, byteorder="little").hex(),
                        f"\nkey:\t\t\t{key_bytes.hex()}\nmessage:\t{pretty_message}",
                    )

    def test_classical_polynomial_kat(self):
        with (
            open(
                f"classical_poly_pi{self.pi}_delta{self.delta}_tagbytes{self.tagsize}_keybytes{self.keysize}_blockbytes{self.blocksize}_KAT_in",
                "r",
            ) as input_values,
            open(
                f"classical_poly_pi{self.pi}_delta{self.delta}_tagbytes{self.tagsize}_keybytes{self.keysize}_blockbytes{self.blocksize}_KAT_out",
                "r",
            ) as output_values,
        ):
            for linenum, (inp, out) in enumerate(zip(input_values, output_values)):
                with self.subTest(linenum=linenum):
                    res = bytes.fromhex(out)
                    k, m = inp.split()
                    message = bytes.fromhex(m)
                    key = bytes.fromhex(k)
                    tag_l = [0] * self.tagsize
                    tag = (ctypes.c_uint8 * len(tag_l))(*tag_l)
                    self.lib.hash(
                        ctypes.pointer(tag),
                        ctypes.pointer((ctypes.c_uint8 * len(message))(*message)),
                        len(message),
                        ctypes.pointer((ctypes.c_uint8 * len(key))(*key)),
                    )
                    self.assertEqual(bytes(tag).hex(), res.hex())


if __name__ == "__main__":
    # unittest.main()
    suite = unittest.TestSuite()
    suite.addTest(
        ClassicalPolynomial(
            "test_classical_polynomial_kat",
            binname="config_reference_implementations_6",
            pi=116,
            delta=3,
            blocksize=14,
            keysize=14,
            tagsize=15,
        )
    )
    unittest.TextTestRunner(verbosity=2).run(suite)
