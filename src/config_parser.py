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
from pathlib import Path
import re
from re import Pattern
from typing import Iterator, Optional, List
from pydantic import TypeAdapter, ValidationError
from src.config_spec import (
    BinaryFieldSpec,
    Config,
    CrandallPrimeFieldSpec,
    FieldSpec,
    FieldTransformSpec,
    KeyGeneratorSpec,
    KeyTransformSpec,
    KeyTransformOptions,
    MersennePrimeFieldSpec,
    MsgTransformSpec,
    MsgTransformOptions,
    MultiplicationSpec,
    NewHashConfig,
    PolynomialSpec,
    ReferenceConfig,
    ConfigurationFile,
    is_supported_lib,
    is_supported_mac,
    is_supported_multiplication,
    is_supported_wordsize,
)
from src.framework_encodings import implicit_bf_encodings


class ParsingError(Exception):
    pass


class LegacyParser:
    _interface_re = r"(?P<blocksize>\d+)\s+(?P<keysize>\d+)\s+(?P<outputsize>\d+)"
    _field_re = (
        r"((?P<PF>pf\s+(?P<prime_type>\d+)\s+(?P<prime_params>(\d+\s+)+))"
        + r"|(?P<BF>bf\s+(?P<field_size>\d+)))\s*(?P<arch>x86|arm)\s+(?P<wordsize>32|64)"
        + r"\s+(?P<num_limbs>\d+)\s+(?P<limb_sizes>(?:\d+\s+)+)\s*(?P<method>schoolbook|karatsuba)"
        + r"(?:\s+(?P<vectorization>AVX|NEON))?"
    )
    _encoding_re = (
        r"(?P<KF>\d+)\s+(clamp\s+(?P<KFclamp>0x[a-fA-F0-9]+\s+))?"
        + r"(?P<MF>\d+)(?P<MFbyte>\s+(low\s+)?0x[a-fA-F0-9]{1,2})?"
        + r"(?P<MFmask>(\s+0x[a-fA-F0-9]{1,16})+)?\s+(?P<FB>\d+)"
    )
    _poly_re = r"(?P<nest_level>\d+)(?P<poly_params>(?:\s+(?:\w+)(?:\s+\d+)*)+)"
    _keygen_re = r"(kg(?P<keygenbytes>\d+))?"
    _name_re = r"((?P<name>\w+))?"
    _config_re: Pattern[str] = re.compile(
        rf"((({_keygen_re})\s+)?({_interface_re})\s+({_field_re})\s+({_encoding_re})"
        + rf"\s+({_poly_re})({_name_re}))"
    )

    def __init__(self, filename: str | Path):
        self.filename = filename
        self.lines: Iterator[tuple[int, str]] = iter([])
        with open(filename, "r", encoding="utf-8") as conf:
            self.lines: Iterator[tuple[int, str]] = iter(enumerate(conf.readlines()))

    def _parse_key_transform(
        self, m: re.Match[str], line_number: int
    ) -> KeyTransformSpec:
        key_transform: KeyTransformSpec = KeyTransformSpec(id=0)
        try:
            key_transform.id = int(m.group("KF").strip())
        except ValueError as exc:
            raise SyntaxError(
                f"Invalid key transform id on line {line_number}"
            ) from exc
        key_clamp: Optional[str] = m.group("KFclamp")
        if key_clamp is not None:
            try:
                key_clamp_mask = int(key_clamp, 16)
            except ValueError as exc:
                error = SyntaxError(
                    f"Invalid key clamping mask format on line {line_number}"
                )
                raise error from exc
            key_transform.options = KeyTransformOptions(mask=key_clamp_mask)
        return KeyTransformSpec.model_validate(key_transform)

    def _parse_msg_transform(
        self, m: re.Match[str], line_number: int
    ) -> MsgTransformSpec:
        msg_transform: MsgTransformSpec = MsgTransformSpec(id=0)
        try:
            msg_transform.id = int(m.group("MF").strip())
        except ValueError as exc:
            raise SyntaxError(
                f"Invalid message transform id on line {line_number}"
            ) from exc
        if msg_transform.id in implicit_bf_encodings:
            if m.group("MFbyte") is not None:
                encodingByte: int = int(m.group("MFbyte").strip().split()[-1], 16)
                lowerEncode: bool = m.group("MFbyte").strip().split()[0] == "low"
            else:
                encodingByte = 0
                lowerEncode = False
            if m.group("MFmask") is not None:
                encodingMask: list[int] = list(
                    map(partial(int, base=16), m.group("MFmask").strip().split())
                )
            else:
                encodingMask = [2**64 - 1]
            msg_transform.options = MsgTransformOptions(
                byte=encodingByte, mask=encodingMask, encodeLSB=lowerEncode
            )
        return MsgTransformSpec.model_validate(msg_transform)

    def _parse_field_transform(
        self, m: re.Match[str], line_number: int
    ) -> FieldTransformSpec:
        field_transform = FieldTransformSpec(id=int(m.group("FB").strip()))
        return field_transform

    def _parse_polynomial(self, m: re.Match[str], line_number: int) -> PolynomialSpec:
        nest_level = int(m.group("nest_level"))
        poly_params: list[str] = re.findall(r"(?:\w+)(?: \d+)*", m.group("poly_params"))
        if len(poly_params) <= nest_level:
            raise SyntaxError("Number of polynomials does not match nest level")
        params: list[str] = poly_params[0].split(" ")
        outerpoly: str = params[0].strip()
        outer_params = list(map(int, params[1:]))
        polynomial = PolynomialSpec(
            name=outerpoly, parameters=outer_params, inner_polynomial=None
        )
        polynomial.parameters = outer_params
        if nest_level == 1:
            params = poly_params[1].split(" ")
            innerpoly: Optional[str] = params[0].strip()
            inner_params = list(map(int, params[1:]))
            polynomial.inner_polynomial = PolynomialSpec(
                name=innerpoly, parameters=inner_params, inner_polynomial=None
            )
        return PolynomialSpec.model_validate(polynomial)

    def _parse_field(self, m: re.Match[str], line_number: int) -> FieldSpec:
        if m.group("PF") is not None:
            prime_type: str = m.group("prime_type").strip()
            params: list[str] = m.group("prime_params").split(" ")
            if prime_type == "0":
                return CrandallPrimeFieldSpec(
                    field_type="crandallprime", pi=int(params[0]), delta=int(params[1])
                )
            if prime_type == "1":
                return MersennePrimeFieldSpec(
                    field_type="mersenneprime", pi=int(params[0])
                )
            raise SyntaxError(f"Unknown prime field type on line {line_number}")
        if m.group("BF") is not None:
            return BinaryFieldSpec(field_type="binary", size=int(m.group("field_size")))
        raise SyntaxError(f"Unknown field type on line {line_number}")

    def _parse_line(self, line: str, line_number: int) -> Config | None:
        if line.strip()[0] == "#":
            return None
        split: list[str] = line.strip().rsplit(maxsplit=1)
        spec: str
        label: str
        spec, label = split[0], split[-1]
        if line.strip()[0:3] == "ref":
            lib: str
            mac: str
            implementation: Optional[str] = None
            params = spec.strip()[3:].strip().split("_", maxsplit=2)
            if len(params) < 2:
                raise SyntaxError(f"Malformed config on line {line_number}")
            lib, mac = params[0], params[1]
            if len(params) == 3:
                implementation = params[2]
            if not is_supported_lib(lib):
                raise SyntaxError(f"Library Specification {lib} is unknown.")
            if not is_supported_mac(mac):
                raise SyntaxError(f"Mac Specifictation {mac} is unknown.")
            return ReferenceConfig(
                name=label, ref=True, lib=lib, mac=mac, implementation=implementation
            )
        m: re.Match[str] | None = self._config_re.match(line)
        if m is None:
            raise SyntaxError(f"Malformed config on line {line_number}")
        keygenerator: KeyGeneratorSpec = KeyGeneratorSpec(required=False)
        if m.group("keygenbytes") is not None:
            keygenerator.number_of_bytes = int(m.group("keygenbytes"))
            keygenerator.required = True
        num_limbs = int(m.group("num_limbs"))
        limbbits: list[int] = []
        for i in range(0, num_limbs):
            params: list[str] = m.group("limb_sizes").split(" ")
            limbbits.append(int(params[i]))
        wordsize = int(m.group("wordsize"))
        multiplication: str = m.group("method").strip()
        if is_supported_wordsize(wordsize) and is_supported_multiplication(
            multiplication
        ):
            result = NewHashConfig(
                name=label,
                blocksize=int(m.group("blocksize")),
                keysize=int(m.group("keysize")),
                tagsize=int(m.group("outputsize")),
                wordsize=wordsize,
                limbs=limbbits,
                multiplication=MultiplicationSpec(
                    method=multiplication, option="precompute"
                ),
                key_transform=self._parse_key_transform(m, line_number),
                msg_transform=self._parse_msg_transform(m, line_number),
                field_transform=self._parse_field_transform(m, line_number),
                polynomial=self._parse_polynomial(m, line_number),
                field=self._parse_field(m, line_number),
                keygenerator=keygenerator,
                ref=False,
            )
            return NewHashConfig.model_validate(result)

    def parse_next_line(self) -> Config | None:
        try:
            line_number: int
            line: str
            line_number, line = next(self.lines)
            result: Config | None = self._parse_line(line, line_number)
        except StopIteration:
            result = None
        return result

    def parse_remaining_lines(self) -> ConfigurationFile:
        configs = [
            line
            for line in [
                self._parse_line(line, line_number) for line_number, line in self.lines
            ]
            if line is not None
        ]
        return ConfigurationFile(name="", configurations=configs)


class ConfigParser:
    def __init__(self, filename: str | Path) -> None:
        self.filename: str | Path = filename

    def parse(self) -> ConfigurationFile:
        with open(self.filename) as f:
            configs: str = f.read()
            try:
                return ConfigurationFile.model_validate_json(configs)
            except ValidationError:
                ta = TypeAdapter(List[Config])
                return ConfigurationFile(
                    name="", configurations=ta.validate_json(configs)
                )
