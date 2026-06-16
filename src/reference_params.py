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
    "haberdashery_gmac_broadwell": {
        "blocksize": 16,
        "keysize": 32 + 12,
        "outputsize": 16,
        "flags": "USE_HABERDASHERY=1",
    },
    "haberdashery_gmac_haswell": {
        "blocksize": 16,
        "keysize": 32 + 12,
        "outputsize": 16,
        "flags": "USE_HABERDASHERY=1",
    },
    "haberdashery_gmac_skylake": {
        "blocksize": 16,
        "keysize": 32 + 12,
        "outputsize": 16,
        "flags": "USE_HABERDASHERY=1",
    },
    "haberdashery_gmac_skylakex": {
        "blocksize": 16,
        "keysize": 32 + 12,
        "outputsize": 16,
        "flags": "USE_HABERDASHERY=1",
    },
    "haberdashery_gmac_tigerlake": {
        "blocksize": 16,
        "keysize": 32 + 12,
        "outputsize": 16,
        "flags": "USE_HABERDASHERY=1",
    },
    "hash2l_hash_128_karatmult3": {
        "blocksize": 16,
        "keysize": 16,
        "outputsize": 16,
        "flags": "USE_HASH2L=1",
    },
    "hash2l_hash_128_sbmult3": {
        "blocksize": 16,
        "keysize": 16,
        "outputsize": 16,
        "flags": "USE_HASH2L=2",
    },
    "hash2l_hash_256_karatmult1": {
        "blocksize": 32,
        "keysize": 32,
        "outputsize": 32,
        "flags": "USE_HASH2L=3",
    },
    "d2lHash_d2lHash1271_d2_g4": {
        "blocksize": 15,
        "keysize": 16,
        "outputsize": 16,
        "flags": "USE_D2LHASH=1 D2LHASHD=d2 D2LHASHG=g4",
    },
    "d2lHash_d2lHash1271_d2_g8": {
        "blocksize": 15,
        "keysize": 16,
        "outputsize": 16,
        "flags": "USE_D2LHASH=1 D2LHASHD=d2 D2LHASHG=g8",
    },
    "d2lHash_d2lHash1271_d3_g4": {
        "blocksize": 15,
        "keysize": 16,
        "outputsize": 16,
        "flags": "USE_D2LHASH=1 D2LHASHD=d3 D2LHASHG=g4",
    },
    "d2lHash_d2lHash1271_d3_g8": {
        "blocksize": 15,
        "keysize": 16,
        "outputsize": 16,
        "flags": "USE_D2LHASH=1 D2LHASHD=d3 D2LHASHG=g8",
    },
    "d2lHash_d2lHash1271_d4_g4": {
        "blocksize": 15,
        "keysize": 16,
        "outputsize": 16,
        "flags": "USE_D2LHASH=1 D2LHASHD=d4 D2LHASHG=g4",
    },
    "d2lHash_d2lHash1271_d4_g8": {
        "blocksize": 15,
        "keysize": 16,
        "outputsize": 16,
        "flags": "USE_D2LHASH=1 D2LHASHD=d4 D2LHASHG=g8",
    },
    "d2lHash_d2lHash1271_d5_g4": {
        "blocksize": 15,
        "keysize": 16,
        "outputsize": 16,
        "flags": "USE_D2LHASH=1 D2LHASHD=d5 D2LHASHG=g4",
    },
    "d2lHash_d2lHash1271_d5_g8": {
        "blocksize": 15,
        "keysize": 16,
        "outputsize": 16,
        "flags": "USE_D2LHASH=1 D2LHASHD=d5 D2LHASHG=g8",
    },
    "d2lHash_d2lHash1305_d2_g4": {
        "blocksize": 16,
        "keysize": 16,
        "outputsize": 16,
        "flags": "USE_D2LHASH=2 D2LHASHD=d2 D2LHASHG=g4",
    },
    "d2lHash_d2lHash1305_d2_g8": {
        "blocksize": 16,
        "keysize": 16,
        "outputsize": 16,
        "flags": "USE_D2LHASH=2 D2LHASHD=d2 D2LHASHG=g8",
    },
    "d2lHash_d2lHash1305_d3_g4": {
        "blocksize": 16,
        "keysize": 16,
        "outputsize": 16,
        "flags": "USE_D2LHASH=2 D2LHASHD=d3 D2LHASHG=g4",
    },
    "d2lHash_d2lHash1305_d3_g8": {
        "blocksize": 16,
        "keysize": 16,
        "outputsize": 16,
        "flags": "USE_D2LHASH=2 D2LHASHD=d3 D2LHASHG=g8",
    },
    "d2lHash_d2lHash1305_d4_g4": {
        "blocksize": 16,
        "keysize": 16,
        "outputsize": 16,
        "flags": "USE_D2LHASH=2 D2LHASHD=d4 D2LHASHG=g4",
    },
    "d2lHash_d2lHash1305_d4_g8": {
        "blocksize": 16,
        "keysize": 16,
        "outputsize": 16,
        "flags": "USE_D2LHASH=2 D2LHASHD=d4 D2LHASHG=g8",
    },
    "d2lHash_d2lHash1305_d5_g4": {
        "blocksize": 16,
        "keysize": 16,
        "outputsize": 16,
        "flags": "USE_D2LHASH=2 D2LHASHD=d5 D2LHASHG=g4",
    },
    "d2lHash_d2lHash1305_d5_g8": {
        "blocksize": 16,
        "keysize": 16,
        "outputsize": 16,
        "flags": "USE_D2LHASH=2 D2LHASHD=d5 D2LHASHG=g8",
    },
    "polyhash_poly1305_g1": {
        "blocksize": 16,
        "keysize": 16,
        "outputsize": 16,
        "flags": "USE_POLYHASH=2 POLYHASHG=g1",
    },
    "polyhash_poly1305_g4": {
        "blocksize": 16,
        "keysize": 16,
        "outputsize": 16,
        "flags": "USE_POLYHASH=1 POLYHASHG=g4",
    },
    "polyhash_poly1305_g8": {
        "blocksize": 16,
        "keysize": 16,
        "outputsize": 16,
        "flags": "USE_POLYHASH=1 POLYHASHG=g8",
    },
    "polyhash_poly1305_g16": {
        "blocksize": 16,
        "keysize": 16,
        "outputsize": 16,
        "flags": "USE_POLYHASH=1 POLYHASHG=g16",
    },
    "polyhash_poly1305_g32": {
        "blocksize": 16,
        "keysize": 16,
        "outputsize": 16,
        "flags": "USE_POLYHASH=1 POLYHASHG=g32",
    },
}
