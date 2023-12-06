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

from typing import TypedDict


class ReferenceParameters(TypedDict):
    blocksize: int
    keysize: int
    outputsize: int
    flags: str


reference_params: dict[str, ReferenceParameters] = {
    "sodium_poly1305": {
        "blocksize": 16,
        "keysize": 32,
        "outputsize": 16,
        "flags": "",
    },
    "openssl_poly1305": {
        "blocksize": 16,
        "keysize": 32,
        "outputsize": 16,
        "flags": "USE_OPEN_SSL=1",
    },
    "openssl_gmac": {
        "blocksize": 16,
        "keysize": 16 + 12,
        "outputsize": 16,
        "flags": "USE_OPEN_SSL=1",
    },
    "haclstar_poly1305": {
        "blocksize": 16,
        "keysize": 32,
        "outputsize": 16,
        "flags": "USE_HACL=1",
    },
    "haclstar_poly1305_128": {
        "blocksize": 16,
        "keysize": 32,
        "outputsize": 16,
        "flags": "USE_HACL=1",
    },
    "haclstar_poly1305_256": {
        "blocksize": 16,
        "keysize": 32,
        "outputsize": 16,
        "flags": "USE_HACL=1",
    },
}
