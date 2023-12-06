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

from typing import Callable, Literal, TypeVar
from math import ceil
from src.util import integer_to_hex, hex_to_integer

T = TypeVar("T")
Transform = Callable[[T], T]
MessageTransform = Callable[[list[str]], list[str]]
KeyTransform = Callable[[int], int]


def identity() -> Transform:
    return lambda x: x


def clamp_key(mask=0) -> KeyTransform:
    return lambda x: x & mask


def append_byte(byte="01") -> MessageTransform:
    return lambda x: [b + byte for b in x]


def append_byte_last(byte="01") -> MessageTransform:
    return lambda x: x[:-1] + [x[-1] + byte] if len(x) > 0 else []


def prepend_bits(byte="01") -> MessageTransform:
    bits = bin(int(byte, 16))[2:]
    return lambda x: [
        integer_to_hex(
            int(bin(hex_to_integer(b))[2:] + bits, 2),
            ceil((len(bits) + hex_to_integer(b).bit_length()) / 8),
        )
        for b in x
    ]


def prepend_bits_last(byte="01") -> MessageTransform:
    bits = bin(int(byte, 16))[2:]
    return lambda x: (
        x[:-1]
        + [
            integer_to_hex(
                int(bin(hex_to_integer(x[-1]))[2:] + bits, 2),
                ceil((len(bits) + hex_to_integer(x[-1]).bit_length()) / 8),
            )
        ]
        if len(x) > 0
        else []
    )


def add_byte(pos: Literal["low", "high"] = "high", byte="01") -> MessageTransform:
    if pos == "low":
        return prepend_bits(byte)
    if pos == "high":
        return append_byte(byte)


def add_byte_last(pos: Literal["low", "high"] = "high", byte="01") -> MessageTransform:
    if pos == "low":
        return prepend_bits_last(byte)
    if pos == "high":
        return append_byte_last(byte)


def length_encode(blocksize) -> MessageTransform:
    return lambda x: (
        x[:-1]
        + [
            x[-1] + "0" * (2 * blocksize - len(x[-1])),
            integer_to_hex(blocksize * len(x) - 1 + len(x[-1]) // 2, blocksize),
        ]
        if len(x) > 0
        else []
    )
