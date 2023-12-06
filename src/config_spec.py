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

from typing import Literal, Optional, TypeGuard
from typing_extensions import Annotated
from pydantic import BaseModel, BeforeValidator, PlainSerializer

HexInt = Annotated[
    int,
    BeforeValidator(lambda x: int(x, base=16) if isinstance(x, str) else x),
    PlainSerializer(hex, return_type=str, when_used="json"),
]


class KeyTransformOptions(BaseModel):
    mask: HexInt


class KeyTransformSpec(BaseModel):
    id: int
    options: Optional[KeyTransformOptions] = None


class MsgTransformOptions(BaseModel):
    byte: HexInt
    mask: list[HexInt]
    encodeLSB: bool


class MsgTransformSpec(BaseModel):
    id: int
    options: Optional[MsgTransformOptions] = None


class FieldTransformSpec(BaseModel):
    id: int


class PolynomialSpec(BaseModel):
    name: str
    parameters: list[int]
    inner_polynomial: Optional["PolynomialSpec"] = None


class CrandallPrimeFieldSpec(BaseModel):
    field_type: Literal["crandallprime"]
    pi: int
    delta: int


class MersennePrimeFieldSpec(BaseModel):
    field_type: Literal["mersenneprime"]
    pi: int


class BinaryFieldSpec(BaseModel):
    field_type: Literal["binary"]
    size: int


PrimeFieldSpec = CrandallPrimeFieldSpec | MersennePrimeFieldSpec
FieldSpec = PrimeFieldSpec | BinaryFieldSpec

ArchSpec = Literal["x86"]
Wordsize = Literal[32, 64]
MultiplicationMethod = Literal["schoolbook", "karatsuba"]
MultiplicationOptions = Literal[
    "precompute", "doublecarry", "doublecarryover", "doublecarrytemp"
]


class MultiplicationSpec(BaseModel):
    method: MultiplicationMethod
    option: Optional[Literal["precompute"]] = None  # deprecated
    options: Optional[list[MultiplicationOptions]] = None


class KeyGeneratorSpec(BaseModel):
    required: bool
    number_of_bytes: Optional[int] = None


class NewHashConfig(BaseModel):
    name: str
    ref: Literal[False]
    keysize: int
    blocksize: int
    tagsize: int
    field: FieldSpec
    wordsize: Wordsize
    limbs: list[int]
    multiplication: MultiplicationSpec
    key_transform: KeyTransformSpec
    msg_transform: MsgTransformSpec
    field_transform: FieldTransformSpec
    polynomial: PolynomialSpec
    keygenerator: KeyGeneratorSpec
    skip: Optional[bool] = False
    description: Optional[str] = ""


class ReferenceConfig(BaseModel):
    name: str
    ref: Literal[True]
    lib: Literal["openssl", "sodium", "haclstar"]
    mac: Literal["poly1305", "gmac"]
    implementation: Optional[str]
    skip: Optional[bool] = False
    description: Optional[str] = ""


Config = NewHashConfig | ReferenceConfig


class ConfigurationFile(BaseModel):
    name: str
    description: Optional[str] = ""
    configurations: list[Config]


def is_supported_lib(
    lib: str,
) -> TypeGuard[Literal["openssl", "sodium", "haclstar"]]:
    return lib in ["openssl", "sodium", "haclstar"]


def is_supported_mac(mac: str) -> TypeGuard[Literal["poly1305", "gmac"]]:
    return mac in ["poly1305", "gmac"]


def is_supported_wordsize(size: int) -> TypeGuard[Wordsize]:
    return size in [32, 64]


def is_supported_arch(arch: str) -> TypeGuard[ArchSpec]:
    return arch in ["x86"]


def is_supported_multiplication(mult: str) -> TypeGuard[MultiplicationMethod]:
    return mult in ["schoolbook", "karatsuba"]


def is_ReferenceConfig(config: Config) -> TypeGuard[ReferenceConfig]:
    return config.ref


def is_NewHashConfig(config: Config) -> TypeGuard[NewHashConfig]:
    return not config.ref


def is_CrandallPrimeFieldSpec(field: FieldSpec) -> TypeGuard[CrandallPrimeFieldSpec]:
    return field.field_type == "crandallprime"


def is_MersennePrimeFieldSpec(field: FieldSpec) -> TypeGuard[MersennePrimeFieldSpec]:
    return field.field_type == "mersenneprime"


def is_BinaryFieldSpec(field: FieldSpec) -> TypeGuard[BinaryFieldSpec]:
    return field.field_type == "binary"


def is_PrimeFieldSpec(field: FieldSpec) -> TypeGuard[PrimeFieldSpec]:
    return is_CrandallPrimeFieldSpec(field) or is_MersennePrimeFieldSpec(field)
