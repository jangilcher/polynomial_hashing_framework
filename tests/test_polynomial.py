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
from math import ceil
from typing import Optional
from typing_extensions import override
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


class Polynomial(unittest.TestCase):
    def __init__(
        self,
        name,
        binname="",
        blocksize=16,
        keysize=16,
        tagsize=16,
        transform: MessageTransform = identity(),
        key_transform: KeyTransform = identity(),
        numtests=1000,
        full_logs=False,
        innerpoly=None,
        superblocksize=None,
        superkeysize=None,
        debug=False,
        stepsize=1,
        hash_transform=None,
        numKeys=1,
        le_extra_key=0,
        le_min_key=0,
    ) -> None:
        super().__init__(name)
        self.blocksize: int = blocksize
        self.keysize: int = keysize
        self.tagsize: int = tagsize
        self.libname: pathlib.Path = pathlib.Path().absolute() / "bin" / f"{binname}.so"
        self.transform: MessageTransform = transform
        self.key_transform: KeyTransform = key_transform
        self.numtests: int = numtests
        self.full_logs: bool = full_logs
        self.innerpoly: Optional[str] = innerpoly
        self.superblocksize: Optional[int] = superblocksize
        self.superkeysize: Optional[int] = superkeysize
        self.debug: bool = debug
        self.stepsize: int = stepsize
        self.hash_transform = hash_transform
        self.numKeys = numKeys
        self.le_extra_key: int = le_extra_key
        self.le_min_key: int = le_min_key

    @abstractmethod
    def getField(self):
        pass

    def setUp(self) -> None:
        self.lib = ctypes.CDLL(self.libname)

    def test_classical_polynomial(self) -> None:
        hash = mod.classical_polynomial(
            field=self.getField(),
            tagbytes=self.tagsize,
            keybytes=self.keysize,
            blockbytes=self.blocksize,
            transform=self.transform,
            key_transform=self.key_transform,
            hash_transform=self.hash_transform,
        )
        self._runTestBattery(hash, keyGen=False)

    def test_DCHM(self) -> None:
        hash = mod.DCHM(
            field=self.getField(),
            tagbytes=self.tagsize,
            keybytes=self.keysize,
            blockbytes=self.blocksize,
            transform=self.transform,
            key_transform=self.key_transform,
            hash_transform=self.hash_transform,
        )
        self._runTestBattery(hash, keyGen=False)

    def test_v1NMH_HORNER(self) -> None:
        inner = mod.__getattribute__(self.innerpoly)(
            field=self.getField(),
            tagbytes=self.tagsize,
            keybytes=self.keysize,
            blockbytes=self.blocksize,
            transform=self.transform,
            key_transform=self.key_transform,
            hash_transform=self.hash_transform,
        )
        hash = mod.v1NMH_HORNER(
            field=self.getField(),
            tagbytes=self.tagsize,
            keybytes=self.keysize,
            blockbytes=self.blocksize,
            transform=self.transform,
            key_transform=self.key_transform,
            hash_transform=self.hash_transform,
            inner_polynomial=inner,
            superblocksize=self.superblocksize // self.blocksize,
            superkeysize=self.superkeysize // self.keysize,
        )
        self._runTestBattery(hash, keyGen=False)

    def test_BRW(self) -> None:
        hash = mod.BRW(
            field=self.getField(),
            tagbytes=self.tagsize,
            keybytes=self.keysize,
            blockbytes=self.blocksize,
            transform=self.transform,
            key_transform=self.key_transform,
            hash_transform=self.hash_transform,
        )
        self._runTestBattery(hash, keyGen=False)

    def test_uSQH(self) -> None:
        hash = mod.uSQH(
            field=self.getField(),
            tagbytes=self.tagsize,
            keybytes=self.keysize,
            blockbytes=self.blocksize,
            transform=self.transform,
            key_transform=self.key_transform,
            hash_transform=self.hash_transform,
        )
        self._runTestBattery(hash, keyGen=False)

    def test_MMH(self) -> None:
        hash = mod.MMH(
            field=self.getField(),
            tagbytes=self.tagsize,
            keybytes=self.keysize,
            blockbytes=self.blocksize,
            transform=self.transform,
            key_transform=self.key_transform,
            hash_transform=self.hash_transform,
        )
        self._runTestBattery(hash, keyGen=False)

    def test_MMH_KG(self) -> None:
        hash = mod.MMH(
            field=self.getField(),
            tagbytes=self.tagsize,
            keybytes=self.keysize,
            blockbytes=self.blocksize,
            transform=self.transform,
            key_transform=self.key_transform,
            hash_transform=self.hash_transform,
        )
        self._runTestBattery(hash, keyGen=True)

    def test_NMH(self) -> None:
        hash = mod.NMH(
            field=self.getField(),
            tagbytes=self.tagsize,
            keybytes=self.keysize,
            blockbytes=self.blocksize,
            transform=self.transform,
            key_transform=self.key_transform,
            hash_transform=self.hash_transform,
        )
        self._runTestBattery(hash, keyGen=False)

    def test_NMH_KG(self) -> None:
        hash = mod.NMH(
            field=self.getField(),
            tagbytes=self.tagsize,
            keybytes=self.keysize,
            blockbytes=self.blocksize,
            transform=self.transform,
            key_transform=self.key_transform,
            hash_transform=self.hash_transform,
        )
        self._runTestBattery(hash, keyGen=True)

    def test_SQH(self) -> None:
        hash = mod.SQH(
            field=self.getField(),
            tagbytes=self.tagsize,
            keybytes=self.keysize,
            blockbytes=self.blocksize,
            transform=self.transform,
            key_transform=self.key_transform,
            hash_transform=self.hash_transform,
        )
        self._runTestBattery(hash, keyGen=False)

    def test_SQH_KG(self) -> None:
        hash = mod.SQH(
            field=self.getField(),
            tagbytes=self.tagsize,
            keybytes=self.keysize,
            blockbytes=self.blocksize,
            transform=self.transform,
            key_transform=self.key_transform,
            hash_transform=self.hash_transform,
        )
        self._runTestBattery(hash, keyGen=True)

    def test_HKM(self) -> None:
        hash = mod.HKM(
            field=self.getField(),
            tagbytes=self.tagsize,
            keybytes=self.keysize,
            blockbytes=self.blocksize,
            transform=self.transform,
            key_transform=self.key_transform,
            hash_transform=self.hash_transform,
        )
        self._runTestBattery(hash, keyGen=False)

    def test_HKM_KG(self) -> None:
        hash = mod.HKM(
            field=self.getField(),
            tagbytes=self.tagsize,
            keybytes=self.keysize,
            blockbytes=self.blocksize,
            transform=self.transform,
            key_transform=self.key_transform,
            hash_transform=self.hash_transform,
        )
        self._runTestBattery(hash, keyGen=True)

    def test_tHorner(self) -> None:
        inner = mod.__getattribute__(self.innerpoly)(
            field=self.getField(),
            tagbytes=self.tagsize,
            keybytes=self.keysize,
            blockbytes=self.blocksize,
            transform=self.transform,
            key_transform=self.key_transform,
            hash_transform=self.hash_transform,
        )
        hash = mod.tHorner(
            field=self.getField(),
            tagbytes=self.tagsize,
            keybytes=self.keysize,
            blockbytes=self.blocksize,
            transform=self.transform,
            key_transform=self.key_transform,
            hash_transform=self.hash_transform,
            inner_polynomial=inner,
            superblocksize=self.superblocksize // self.blocksize,
            superkeysize=self.superkeysize // self.keysize,
        )
        self._runTestBattery(hash, keyGen=False)

    def test_tMMH(self) -> None:
        inner = mod.__getattribute__(self.innerpoly)(
            field=self.getField(),
            tagbytes=self.tagsize,
            keybytes=self.keysize,
            blockbytes=self.blocksize,
            transform=self.transform,
            key_transform=self.key_transform,
            hash_transform=self.hash_transform,
        )
        hash = mod.tMMH(
            field=self.getField(),
            tagbytes=self.tagsize,
            keybytes=self.keysize,
            blockbytes=self.blocksize,
            transform=self.transform,
            key_transform=self.key_transform,
            hash_transform=self.hash_transform,
            inner_polynomial=inner,
            superblocksize=self.superblocksize // self.blocksize,
            superkeysize=self.superkeysize // self.keysize,
        )
        self._runTestBattery(hash, keyGen=True)

    def test_tSQH(self) -> None:
        inner = mod.__getattribute__(self.innerpoly)(
            field=self.getField(),
            tagbytes=self.tagsize,
            keybytes=self.keysize,
            blockbytes=self.blocksize,
            transform=self.transform,
            key_transform=self.key_transform,
            hash_transform=self.hash_transform,
        )
        hash = mod.tSQH(
            field=self.getField(),
            tagbytes=self.tagsize,
            keybytes=self.keysize,
            blockbytes=self.blocksize,
            transform=self.transform,
            key_transform=self.key_transform,
            hash_transform=self.hash_transform,
            inner_polynomial=inner,
            superblocksize=self.superblocksize // self.blocksize,
            superkeysize=self.superkeysize // self.keysize,
        )
        self._runTestBattery(hash, keyGen=True)

    def test_tNMH(self) -> None:
        inner = mod.__getattribute__(self.innerpoly)(
            field=self.getField(),
            tagbytes=self.tagsize,
            keybytes=self.keysize,
            blockbytes=self.blocksize,
            transform=self.transform,
            key_transform=self.key_transform,
            hash_transform=self.hash_transform,
        )
        hash = mod.tNMH(
            field=self.getField(),
            tagbytes=self.tagsize,
            keybytes=self.keysize,
            blockbytes=self.blocksize,
            transform=self.transform,
            key_transform=self.key_transform,
            hash_transform=self.hash_transform,
            inner_polynomial=inner,
            superblocksize=self.superblocksize // self.blocksize,
            superkeysize=self.superkeysize // self.keysize,
        )
        self._runTestBattery(hash, keyGen=True)

    def test_tHKM(self) -> None:
        inner = mod.__getattribute__(self.innerpoly)(
            field=self.getField(),
            tagbytes=self.tagsize,
            keybytes=self.keysize,
            blockbytes=self.blocksize,
            transform=self.transform,
            key_transform=self.key_transform,
            hash_transform=self.hash_transform,
        )
        hash = mod.tHKM(
            field=self.getField(),
            tagbytes=self.tagsize,
            keybytes=self.keysize,
            blockbytes=self.blocksize,
            transform=self.transform,
            key_transform=self.key_transform,
            hash_transform=self.hash_transform,
            inner_polynomial=inner,
            superblocksize=self.superblocksize // self.blocksize,
            superkeysize=self.superkeysize // self.keysize,
        )
        self._runTestBattery(hash, keyGen=True)

    def test_tBRW(self) -> None:
        inner = mod.__getattribute__(self.innerpoly)(
            field=self.getField(),
            tagbytes=self.tagsize,
            keybytes=self.keysize,
            blockbytes=self.blocksize,
            transform=self.transform,
            key_transform=self.key_transform,
            hash_transform=self.hash_transform,
        )
        hash = mod.tBRW(
            field=self.getField(),
            tagbytes=self.tagsize,
            keybytes=self.keysize,
            blockbytes=self.blocksize,
            transform=self.transform,
            key_transform=self.key_transform,
            hash_transform=self.hash_transform,
            inner_polynomial=inner,
            superblocksize=self.superblocksize // self.blocksize,
            superkeysize=self.superkeysize // self.keysize,
        )
        self._runTestBattery(hash, keyGen=True)

    def test_MHP(self) -> None:
        inner = mod.__getattribute__(self.innerpoly)(
            field=self.getField(),
            tagbytes=self.tagsize,
            keybytes=self.keysize,
            blockbytes=self.blocksize,
            transform=self.transform,
            key_transform=self.key_transform,
            hash_transform=self.hash_transform,
        )
        superkeysize = self.superkeysize // self.keysize
        hash = mod.MHP(
            field=self.getField(),
            tagbytes=self.tagsize,
            keybytes=self.keysize,
            blockbytes=self.blocksize,
            transform=self.transform,
            key_transform=self.key_transform,
            hash_transform=self.hash_transform,
            inner_polynomial=inner,
            superblocksize=self.superblocksize // self.blocksize,
            superkeysize=superkeysize,
        )
        numKeys: int = self.numKeys + superkeysize
        if numKeys < self.le_min_key:
            numKeys = self.le_min_key
        else:
            numKeys += self.le_extra_key
        self._runTestBattery(hash, keyGen=False, numKeys=numKeys)

    def test_d2LHP(self) -> None:
        inner = mod.__getattribute__(self.innerpoly)(
            field=self.getField(),
            tagbytes=self.tagsize,
            keybytes=self.keysize,
            blockbytes=self.blocksize,
            transform=self.transform,
            key_transform=self.key_transform,
            hash_transform=self.hash_transform,
        )
        superkeysize = self.superkeysize // self.keysize
        hash = mod.d2LHP(
            field=self.getField(),
            tagbytes=self.tagsize,
            keybytes=self.keysize,
            blockbytes=self.blocksize,
            transform=self.transform,
            key_transform=self.key_transform,
            hash_transform=self.hash_transform,
            inner_polynomial=inner,
            superblocksize=self.superblocksize // self.blocksize,
            superkeysize=superkeysize,
        )
        numKeys: int = self.numKeys + superkeysize
        if numKeys < self.le_min_key:
            numKeys = self.le_min_key
        else:
            numKeys += self.le_extra_key
        self._runTestBattery(hash, keyGen=False, numKeys=numKeys)

    def _getRandomKey(
        self, message, keyGen: bool = False, numKeys=1
    ) -> tuple[bytes, int]:
        if keyGen:
            keylength = self.lib.get_keylength(len(message))
        else:
            keylength = self.keysize * numKeys
        if keylength > 0:
            key = random.randrange(0, 2 ** (8 * keylength))
            key_bytes = key.to_bytes(keylength, byteorder="little")
        else:
            key_bytes = bytes()
        return key_bytes, keylength

    def _getOneKey(self, message, keyGen: bool = False, numKeys=1) -> tuple[bytes, int]:
        if keyGen:
            keylength = self.lib.get_keylength(len(message))
        else:
            keylength = self.keysize * numKeys
        key = ("01" + "00" * (self.keysize - 1)) * ceil(keylength / self.keysize)
        key_bytes = bytes.fromhex(key)
        return key_bytes, keylength

    def _getRandomMessage(self, messagesize) -> bytes:
        if messagesize == 0:
            message = bytes()
        else:
            message = random.randrange(0, 2 ** (8 * messagesize)).to_bytes(
                messagesize, byteorder="little"
            )
        return message

    def _getOneMessage(self, messagesize) -> bytes:
        if messagesize == 0:
            message = bytes()
        else:
            message_str = ("01" + "00" * (self.blocksize - 1)) * (
                messagesize // self.blocksize
            )
            if messagesize % self.blocksize:
                message_str += "01" + "00" * ((messagesize % self.blocksize) - 1)
            message = bytes.fromhex(message_str)
        return message

    def _runOnce(self, hash_fun, message, messagesize, key_bytes, keylength) -> None:
        maxPrintSize: int = 128
        res = hash_fun.eval(message.hex(), key_bytes.hex())
        tag_l = [0] * self.tagsize
        tag = (ctypes.c_uint8 * len(tag_l))(*tag_l)
        self.lib.hash(
            ctypes.pointer(tag),
            (ctypes.c_uint8 * len(message))(*message),
            len(message),
            (ctypes.c_uint8 * len(key_bytes))(*key_bytes),
            len(key_bytes),
        )
        pretty_message = (
            message.hex()[: (maxPrintSize // 2 - 1) * 2]
            + ".." * (2 + maxPrintSize % 2)
            + message.hex()[-(maxPrintSize // 2 - 1) * 2 :]
            if messagesize > maxPrintSize and not self.full_logs
            else message.hex()
        )
        pretty_key = (
            key_bytes.hex()[: (maxPrintSize // 2 - 1) * 2]
            + ".." * (2 + maxPrintSize % 2)
            + key_bytes.hex()[-(maxPrintSize // 2 - 1) * 2 :]
            if keylength > maxPrintSize and not self.full_logs
            else key_bytes.hex()
        )
        self.assertEqual(
            bytes(tag).hex(),
            int(res).to_bytes(self.tagsize, byteorder="little").hex(),
            f'expected {int(res).to_bytes(self.tagsize, byteorder="little").hex()}\n'
            + f"\nkey:\t\t\t{pretty_key}\nmessage:\t{pretty_message}",
        )

    def _runOneMessageRange(
        self,
        hash_fun,
        maxMessageSize,
        keyGen: bool = False,
        numKeys=1,
    ) -> None:
        for messagesize in range(0, maxMessageSize, self.stepsize):
            for it in range(self.numtests):
                with self.subTest(
                    messagesize=messagesize,
                    iteration=it,
                    msg="One element Message, Random Key",
                ):
                    message = self._getOneMessage(messagesize=messagesize)
                    key_bytes, keylength = self._getRandomKey(
                        message, keyGen=keyGen, numKeys=numKeys
                    )
                    self._runOnce(
                        hash_fun,
                        message=message,
                        messagesize=messagesize,
                        key_bytes=key_bytes,
                        keylength=keylength,
                    )

    def _runOneKeyRange(
        self, hash_fun, maxMessageSize, keyGen: bool = False, numKeys=1
    ) -> None:
        for messagesize in range(0, maxMessageSize, self.stepsize):
            for it in range(self.numtests):
                with self.subTest(
                    messagesize=messagesize,
                    iteration=it,
                    msg="Random Message, One element Key",
                ):
                    message = self._getRandomMessage(messagesize=messagesize)
                    key_bytes, keylength = self._getOneKey(
                        message,
                        keyGen=keyGen,
                        numKeys=numKeys,
                    )
                    self._runOnce(
                        hash_fun,
                        message=message,
                        messagesize=messagesize,
                        key_bytes=key_bytes,
                        keylength=keylength,
                    )

    def _runOneKeyOneMessageRange(
        self,
        hash_fun,
        maxMessageSize,
        keyGen: bool = False,
        numKeys=1,
    ) -> None:
        for messagesize in range(0, maxMessageSize, self.stepsize):
            for it in range(self.numtests):
                with self.subTest(
                    messagesize=messagesize,
                    iteration=it,
                    msg="One Element Message and Key",
                ):
                    message = self._getOneMessage(messagesize=messagesize)
                    key_bytes, keylength = self._getOneKey(
                        message,
                        keyGen=keyGen,
                        numKeys=numKeys,
                    )
                    self._runOnce(
                        hash_fun,
                        message=message,
                        messagesize=messagesize,
                        key_bytes=key_bytes,
                        keylength=keylength,
                    )

    def _runFuzzRange(
        self,
        hash_fun,
        maxMessageSize,
        keyGen: bool = False,
        numKeys=1,
    ) -> None:
        for messagesize in range(0, maxMessageSize, self.stepsize):
            for it in range(self.numtests):
                with self.subTest(
                    messagesize=messagesize, iteration=it, msg="Random Message"
                ):
                    message = self._getRandomMessage(messagesize=messagesize)
                    key_bytes, keylength = self._getRandomKey(
                        message,
                        keyGen=keyGen,
                        numKeys=numKeys,
                    )
                    self._runOnce(
                        hash_fun,
                        message=message,
                        messagesize=messagesize,
                        key_bytes=key_bytes,
                        keylength=keylength,
                    )

    def _runFuzzLarge(
        self,
        hash_fun,
        keyGen: bool = False,
        numKeys=1,
    ) -> None:
        for messagesize in [2**13]:
            for it in range(self.numtests):
                with self.subTest(
                    messagesize=messagesize, iteration=it, msg="Large Random Message"
                ):
                    message = self._getRandomMessage(messagesize=messagesize)
                    key_bytes, keylength = self._getRandomKey(
                        message,
                        keyGen=keyGen,
                        numKeys=numKeys,
                    )
                    self._runOnce(
                        hash_fun,
                        message=message,
                        messagesize=messagesize,
                        key_bytes=key_bytes,
                        keylength=keylength,
                    )

    def _runTestBattery(self, hash_fun, keyGen: bool = False, numKeys=1) -> None:
        if self.superblocksize is None:
            maxMessageSize: int = self.blocksize * 16 + 1
        else:
            maxMessageSize: int = self.superblocksize * 16 + 1
        if self.debug:
            self._runOneKeyOneMessageRange(
                hash_fun,
                maxMessageSize=maxMessageSize,
                keyGen=keyGen,
                numKeys=numKeys,
            )
            self._runOneMessageRange(
                hash_fun,
                maxMessageSize=maxMessageSize,
                keyGen=keyGen,
                numKeys=numKeys,
            )
            self._runOneKeyRange(
                hash_fun,
                maxMessageSize=maxMessageSize,
                keyGen=keyGen,
                numKeys=numKeys,
            )
        self._runFuzzRange(
            hash_fun,
            maxMessageSize=maxMessageSize,
            keyGen=keyGen,
            numKeys=numKeys,
        )
        self._runFuzzLarge(
            hash_fun,
            keyGen=keyGen,
            numKeys=numKeys,
        )


class PfPolynomial(Polynomial):
    def __init__(
        self,
        *args,
        primetype="0",
        pi=130,
        delta=5,
        **kwargs,
    ) -> None:
        super().__init__(*args, **kwargs)
        self.pi: int = pi
        self.delta: int = delta

    @override
    def getField(self):
        return mod.PrimeField(
            pi=self.pi,
            delta=self.delta,
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


class BfPolynomial(Polynomial):
    def __init__(
        self,
        *args,
        fieldsize=128,
        polynomial=[128, 7, 2, 1, 0],
        **kwargs,
    ) -> None:
        super().__init__(*args, **kwargs)
        self.fieldsize: int = fieldsize
        self.polynomial: list[int] = polynomial

    @override
    def getField(self):
        return mod.BinaryField(
            fieldsize=self.fieldsize,
            polynomial=self.polynomial,
        )


if __name__ == "__main__":
    # unittest.main()
    suite = unittest.TestSuite()
    suite.addTest(
        PfPolynomial(
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
