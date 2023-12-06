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

from functools import partial
from typing import Callable
from tests.transform import (
    Transform,
    MessageTransform,
    KeyTransform,
    identity,
    add_byte,
    add_byte_last,
    length_encode,
    clamp_key,
    append_byte,
)

# add new transforms here:
message_to_field_encoding: dict[
    int, tuple[str, str, Callable[..., MessageTransform]]
] = {
    0: ("identity_transform", "identity", identity),
    1: ("identity_inline_transform", "identity_inline", identity),
    2: ("identity_processBlocksize_transform", "identity_processBlocksize", identity),
    # implicit encoding with a byte and a mask
    3: ("none_transform", "none", add_byte),
    # implicit encoding with a byte and a mask for only the last block
    4: ("none_transform", "none", add_byte_last),
    # length encoding in the last block(s),
    # NOT IMPLEMENTED!
    5: ("none_transform", "none", length_encode),
    # append 0x01 to message block
    10: ("append_bit_transform", "append_bit", append_byte),
}

key_to_field_encoding: dict[int, tuple[str, str, Callable[..., KeyTransform]]] = {
    0: ("identity_transform", "identity", identity),
    1: ("identity_inline_transform", "identity_inline", identity),
    # clamp lower 2 bits and higher 4 bits
    6: (
        "key_clamping_transform",
        "key_clamping",
        partial(clamp_key, 0x0FFFFFFC0FFFFFFC0FFFFFFC0FFFFFFC),
    ),
    # encode message with cleared 2 lower bits and 2 higher bits
    7: (
        "msg_enc_clamping2_transform",
        "msg_enc_clamping2",
        partial(clamp_key, 0xFFFFFFC0FFFFFFC0FFFFFFC0FFFFFFF),
    ),
    # encode message with cleared 2 lower bits and 6 higher bits
    8: (
        "msg_enc_clamping4_transform",
        "msg_enc_clamping4",
        partial(clamp_key, 0xFFFFFFC0FFFFFFC0FFFFFFC0FFFFFFF),
    ),
    # same clamping as in original Poly1305 (2 lower bits and 4 higher bits)
    9: (
        "key_clamping_poly1305_transform",
        "key_clamping_poly1305",
        partial(clamp_key, 0x0FFFFFFC0FFFFFFC0FFFFFFC0FFFFFFF),
    ),
    # generic key clamping
    11: ("none", "none", clamp_key),
    # clamp higher 4 bits
    12: (
        "key_clamping_up_transform",
        "key_clamping_up",
        partial(clamp_key, 0x0FFFFFFF0FFFFFFF0FFFFFFF0FFFFFFF0FFFFFFF0FFFFFFF),
    ),
}

bit_to_field_encoding: dict[int, tuple[str, str, Callable[..., Transform]]] = {
    0: ("identity_transform", "identity", identity),
    1: ("identity_inline_transform", "identity_inline", identity),
    2: ("identity_processBlocksize_transform", "identity_processBlocksize", identity),
    # implicit encoding with a byte and a mask
    3: ("none_transform", "none", add_byte),
    # implicit encoding with a byte and a mask for only the last block
    4: ("none_transform", "none", add_byte_last),
    # length encoding in the last block(s),
    # NOT IMPLEMENTED!
    5: ("none_transform", "none", length_encode),
    # clamp lower 2 bits and higher 4 bits
    6: (
        "key_clamping_transform",
        "key_clamping",
        partial(clamp_key, 0x0FFFFFFC0FFFFFFC0FFFFFFC0FFFFFFC),
    ),
    # encode message with cleared 2 lower bits and 2 higher bits
    7: (
        "msg_enc_clamping2_transform",
        "msg_enc_clamping2",
        partial(clamp_key, 0xFFFFFFC0FFFFFFC0FFFFFFC0FFFFFFF),
    ),
    # encode message with cleared 2 lower bits and 6 higher bits
    8: (
        "msg_enc_clamping4_transform",
        "msg_enc_clamping4",
        partial(clamp_key, 0xFFFFFFC0FFFFFFC0FFFFFFC0FFFFFFF),
    ),
    # same clamping as in original Poly1305 (2 lower bits and 4 higher bits)
    9: (
        "key_clamping_poly1305_transform",
        "key_clamping_poly1305",
        partial(clamp_key, 0x0FFFFFFC0FFFFFFC0FFFFFFC0FFFFFFF),
    ),
    # append 0x01 to message block
    10: ("append_bit_transform", "append_bit", append_byte),
    # generic key clamping
    11: ("none", "none", clamp_key),
    # clamp higher 4 bits
    12: (
        "key_clamping_up_transform",
        "key_clamping_up",
        partial(clamp_key, 0x0FFFFFFF0FFFFFFF0FFFFFFF0FFFFFFF0FFFFFFF0FFFFFFF),
    ),
}

implicit_bf_encodings: list[int] = [
    3,
    4,
    5,
    11,
]
field_to_bit_encoding: dict[int, tuple[str, str]] = {
    0: ("identity_transform", "identity"),
    1: ("identity_inline_transform", "identity_inline"),
}
