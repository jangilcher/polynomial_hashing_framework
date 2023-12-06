#!/usr/bin/env python3
#
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

import getopt
import os
import sys
import subprocess
import unittest
from warnings import warn
from math import ceil
from pathlib import Path
from typing import Callable, Optional

from cpuinfo import get_cpu_info

from tests.test_binary_field_arithmetic import TestArith as BFTestArith
from tests.test_prime_field_arithmetic import TestArith as PFTestArith
from tests.transform import MessageTransform
import src.plot_results as pltrs
from src.settings import Settings
from src.field_arithmetic.bf_polynomial_coeffs import polynomials
from src.reference_params import reference_params
from src.field_arithmetic.generate_field_arithmetic import (
    BinaryFieldArithmeticGenerator,
    MersenneArithmeticGenerator,
    CrandallArithmeticGenerator,
    PrecomputingCrandallArithmeticGenerator,
)
from src.framework_encodings import (
    implicit_bf_encodings,
    field_to_bit_encoding,
    key_to_field_encoding,
    message_to_field_encoding,
)
from src.config_parser import LegacyParser, ConfigParser, ParsingError
from src.config_spec import (
    ConfigurationFile,
    FieldSpec,
    MultiplicationOptions,
    is_PrimeFieldSpec,
    is_ReferenceConfig,
    is_NewHashConfig,
    is_MersennePrimeFieldSpec,
    is_CrandallPrimeFieldSpec,
    is_BinaryFieldSpec,
)
from src.util import integer_to_hex

opts: list[tuple[str, str]]
config_files: list[str]
opts, config_files = getopt.getopt(
    sys.argv[1:],
    "",
    [
        "no_build",
        "no_bench",
        "no_plot",
        "no_test",
        "verbose",
        "tune",
        "debug",
        "plot_compare_only",
        "no_plot_titles",
        "show_plots",
        "check_overflow",
        "no_test_arith",
        "convert_only",
        "convert",
        "latex",
        "full_logs",
        "ctgrind",
        "ctgrind_bin=",
        "iterations=",
        "max_messagesize=",
        "stepsize=",
        "include=",
        "numtests=",
    ],
)

settings: Settings = Settings.from_options(opts)


def green(s: str) -> str:
    return "\033[92m" + s + "\033[0m"


def yellow(s: str) -> str:
    return "\033[93m" + s + "\033[0m"


def red(s: str) -> str:
    return "\033[91m" + s + "\033[0m"


try:
    import sage

    os.system("sage --preparse tests/polynomial.sage")
    settings.sage = True
except ModuleNotFoundError:
    warn(yellow("Could not find Sage. Full integration tests will be skipped!"))
finally:
    from tests.test_polynomial import *

if len(config_files) == 0:
    config_files.append("config")
benchdir = "bench/"
binname: str = ""
arithmetic_test_results: dict[str, tuple[unittest.TestResult, str]] = {}
hash_test_results: dict[str, tuple[unittest.TestResult, str]] = {}
ctgrind_results: dict[str, tuple[subprocess.CompletedProcess, str]] = {}
configs: list[tuple[ConfigurationFile, Path]] = []
for config_file in config_files:
    file = Path(config_file)
    try:
        if file.suffix == ".json":
            parser = ConfigParser(file)
            configs.append((parser.parse(), file))
        else:
            parser = LegacyParser(file)
            configs.append((parser.parse_remaining_lines(), file))
    except Exception as err:
        raise ParsingError(f"Error during parsing of {file}.") from err


if settings.convert:
    for config, file in configs:
        if file.suffix != ".json":
            with open(f"{file}.json", "w") as f:
                f.write(config.model_dump_json(indent=4))

if settings.convert_only:
    sys.exit(0)

os.system("make clean")
for config, file in configs:
    print(f"Starting with {file}")
    linenums: list[int] = []
    labels: list[str] = []
    for config_number, current_config in enumerate(config.configurations):
        if current_config.skip:
            print(f"Skipping config: {file}:{config_number}: {current_config.name}")
            continue
        make_cmd: list[str] = ["make"]
        hash_TestSuite = unittest.TestSuite()
        arithmetic_TestSuite = unittest.TestSuite()
        ccflag: str = ""
        if settings.tune:
            ccflag += "-mtune=native"
        print(f"Working on config: {file}:{config_number}: {current_config.name}")
        linenums.append(config_number)
        make_cmd.append(f"BENCHDIR={benchdir}")
        macro_defs: list[str] = []
        ref = False
        macro_defs.append(f"-DREPETITIONS={settings.iterations}")
        macro_defs.append(f"-DMAXINPUTSIZE={settings.max_message_size}")
        macro_defs.append(f"-DSTEPSIZE={settings.stepsize}")
        if settings.ctgrind:
            make_cmd.append("USE_CTGRIND=0")
        additional_includes = map(
            lambda s: f"obj/{file.name}_{config_number}/" + s[:-2] + ".o",
            settings.includes,
        )
        make_cmd.append('ADDDEPS="' + " ".join(additional_includes) + '"')
        if is_ReferenceConfig(current_config):
            labels.append(current_config.name)
            ref = True
            key: str = f"{current_config.lib}_{current_config.mac}"
            if current_config.implementation is not None:
                key += f"_{current_config.implementation}"
            macro_defs.append(f'-DBLOCKSIZE={reference_params[key]["blocksize"]}')
            macro_defs.append(f'-DKEYSIZE={reference_params[key]["keysize"]}')
            macro_defs.append(f'-DOUTPUTSIZE={reference_params[key]["outputsize"]}')
            binname = key
            make_cmd.append(f'MDEFS="{" ".join(macro_defs)}"')
            make_cmd.append(f"BINNAME={binname}")
            make_cmd.append(f"BENCHRESNAME={file.name}_{config_number}")
            make_cmd.append(reference_params[key]["flags"])
        elif is_NewHashConfig(current_config):
            binname = f"{file.name}_{config_number}"
            if current_config.keygenerator.required:
                if current_config.keygenerator.number_of_bytes is not None:
                    macro_defs.append(
                        f"-DMAX_RAND_BYTES={current_config.keygenerator.number_of_bytes}"
                    )
            macro_defs.append(f"-DBLOCKSIZE={current_config.blocksize}")
            macro_defs.append(f"-DKEYSIZE={current_config.keysize}")
            macro_defs.append(f"-DOUTPUTSIZE={current_config.tagsize}")
            wordsize = current_config.wordsize
            num_limbs: int = len(current_config.limbs)
            limbbits: list[int] = current_config.limbs

            method: str = current_config.multiplication.method
            if method == "karatsuba":
                raise NotImplementedError("Karatsuba currently not supported")
            key_enc_id: int = current_config.key_transform.id
            key_encoding: str
            key_include: str
            key_clamp_mask: int = 2 ** (current_config.keysize * 8) - 1
            if current_config.key_transform.options is not None:
                key_clamp_mask = current_config.key_transform.options.mask
            key_proto_transform: Callable[..., KeyTransform]
            key_encoding, key_include, key_proto_transform = key_to_field_encoding[
                key_enc_id
            ]
            if key_enc_id in implicit_bf_encodings:
                key_transform: KeyTransform = key_proto_transform(key_clamp_mask)
            else:
                key_transform = key_proto_transform()
            make_cmd.append(f'KEYINCLUDE="{key_include}" KEYTRANSFORM={key_encoding}')
            message_enc_id: int = current_config.msg_transform.id
            explicitEncoding: bool = True
            message_encoding: str
            message_include: str
            message_proto_transform: Callable[..., MessageTransform]
            (
                message_encoding,
                message_include,
                message_proto_transform,
            ) = message_to_field_encoding[message_enc_id]
            message_transform: MessageTransform
            if message_enc_id in implicit_bf_encodings:
                explicitEncoding = False
                if current_config.msg_transform.options:
                    encodingMSB: int = current_config.msg_transform.options.byte
                    lowerEncode: bool = current_config.msg_transform.options.encodeLSB
                    encodingMask: list[int] = current_config.msg_transform.options.mask
                else:
                    raise SyntaxError(f"Invalid config in line {config_number}")
                if len(encodingMask) != num_limbs:
                    if len(encodingMask) == 1:
                        encodingMask *= num_limbs
                    else:
                        raise SyntaxError(f"Malformed config on line {config_number}")
                message_transform = message_proto_transform(
                    byte=integer_to_hex(encodingMSB, 1),
                    pos="low" if lowerEncode else "high",
                )
            else:
                encodingMSB = 0
                encodingMask = [
                    int("0xffffffff" if wordsize == 32 else "0xffffffffffffffff", 16)
                ] * num_limbs
                lowerEncode = False
                message_transform = message_proto_transform()
            if message_enc_id == 4:
                lastOnlyEnc: bool = True
            else:
                lastOnlyEnc = False
            make_cmd.append(
                f'MSGINCLUDE="{message_include}" MSGTRANSFORM={message_encoding}'
            )
            field_elem_to_bits: str
            fb_include: str
            (field_elem_to_bits, fb_include) = field_to_bit_encoding[
                current_config.field_transform.id
            ]
            make_cmd.append(
                f'FIELDELEMINCLUDE="{fb_include}" FIELDELEMTRANSFORM={field_elem_to_bits}'
            )

            outerpoly: str = current_config.polynomial.name
            make_cmd.append(f'OUTERPOLY={outerpoly} OUTERPOLY_H="{outerpoly}.h"')
            for i, p in enumerate(current_config.polynomial.parameters):
                macro_defs.append(f"-DOUTER_PARAM{i}={p}")
            if current_config.polynomial.inner_polynomial is not None:
                innerpoly: Optional[
                    str
                ] = current_config.polynomial.inner_polynomial.name
                make_cmd.append(f'INNERPOLY={innerpoly} INNERPOLY_H="{innerpoly}.h"')
                for i, p in enumerate(
                    current_config.polynomial.inner_polynomial.parameters
                ):
                    macro_defs.append(f"-DINNER_PARAM{i}={p}")
                if (
                    current_config.polynomial.inner_polynomial.inner_polynomial
                    is not None
                ):
                    raise NotImplementedError("Nesting level >= 2 not supported")
            else:
                innerpoly = None
            labels.append(current_config.name)
            field: FieldSpec = current_config.field
            multiplication_options: list[
                MultiplicationOptions
            ] | None = current_config.multiplication.options
            if multiplication_options is None:
                multiplication_options = []
            if is_PrimeFieldSpec(field):
                if is_CrandallPrimeFieldSpec(field):
                    prime_type: str = "0"
                    pi: int = field.pi
                    if current_config.blocksize * 8 + encodingMSB.bit_length() > pi:
                        w = "Encoding incompatible with chosen field size.\n"
                        w += f"Need {current_config.blocksize*8 + encodingMSB.bit_length()} bit "
                        w += f"to encode {encodingMSB:x} with blocksize {current_config.blocksize}."
                        w += f" Field size is {pi}."
                        warn(red(w))
                    delta: int = field.delta
                    buffsize: int = ceil(pi / wordsize) * 8
                    macro_defs.append(f"-DBUFFSIZE={buffsize}")
                    print("generating Field Arithmetic")
                    with open(
                        f"src/field_arithmetic/pf_arithmetic_{prime_type}_"
                        + f"{pi}_{delta}_"
                        + "_".join(map(str, current_config.limbs))
                        + f"_{current_config.wordsize}_{method}.h",
                        "w",
                        encoding="utf-8",
                    ) as outfile:
                        if (
                            current_config.multiplication.option == "precompute"
                            or "precompute" in multiplication_options
                        ):
                            make_cmd.append("PC=_pc")
                            arithGen = PrecomputingCrandallArithmeticGenerator(
                                pi,
                                delta,
                                limbbits,
                                num_limbs,
                                wordsize,
                                buffsize,
                                file=outfile,
                                blocksize=current_config.blocksize,
                                keysize=current_config.keysize,
                                encodingMSB=encodingMSB,
                                lowerEncode=lowerEncode,
                                lastOnlyEnc=lastOnlyEnc,
                                encodingMask=encodingMask,
                                explicitEncoding=explicitEncoding,
                                nocheck=not settings.check_overflow,
                                doublecarry="doublecarry" in multiplication_options,
                                doublecarryover="doublecarryover"
                                in multiplication_options,
                                doublecarry_temp="doublecarrytemp"
                                in multiplication_options,
                            )
                        else:
                            make_cmd.append("PC=")
                            arithGen = CrandallArithmeticGenerator(
                                pi,
                                delta,
                                limbbits,
                                num_limbs,
                                wordsize,
                                buffsize,
                                file=outfile,
                                blocksize=current_config.blocksize,
                                keysize=current_config.keysize,
                                encodingMSB=encodingMSB,
                                lowerEncode=lowerEncode,
                                lastOnlyEnc=lastOnlyEnc,
                                encodingMask=encodingMask,
                                explicitEncoding=explicitEncoding,
                                nocheck=not settings.check_overflow,
                                doublecarry="doublecarry" in multiplication_options,
                                doublecarryover="doublecarryover"
                                in multiplication_options,
                                doublecarry_temp="doublecarrytemp"
                                in multiplication_options,
                            )
                        arithGen.print_fieldmul()
                    make_cmd.append(
                        f"PRIMETYPE={prime_type} PI={pi} DELTA={delta} PRIMENAME={pi}_{delta}"
                    )
                    make_cmd.append("pf_arithmetic")
                    if settings.test_arith:
                        arithmetic_TestSuite.addTests(
                            [
                                PFTestArith(
                                    name="test_pack",
                                    binname=binname,
                                    pi=pi,
                                    delta=delta,
                                    wordsize=wordsize,
                                    limbsizes=limbbits,
                                    method=method,
                                    primetype=prime_type,
                                    primename=f"{pi}_{delta}",
                                    blocksize=current_config.blocksize,
                                    keysize=current_config.keysize,
                                    iterations=settings.numtests,
                                ),
                                PFTestArith(
                                    name="test_unpack_key",
                                    binname=binname,
                                    pi=pi,
                                    delta=delta,
                                    wordsize=wordsize,
                                    limbsizes=limbbits,
                                    method=method,
                                    primetype=prime_type,
                                    primename=f"{pi}_{delta}",
                                    blocksize=current_config.blocksize,
                                    keysize=current_config.keysize,
                                    iterations=settings.numtests,
                                ),
                                PFTestArith(
                                    name="test_unpack_msg",
                                    binname=binname,
                                    pi=pi,
                                    delta=delta,
                                    wordsize=wordsize,
                                    limbsizes=limbbits,
                                    method=method,
                                    primetype=prime_type,
                                    primename=f"{pi}_{delta}",
                                    blocksize=current_config.blocksize,
                                    keysize=current_config.keysize,
                                    iterations=settings.numtests,
                                ),
                                PFTestArith(
                                    name="test_mul",
                                    binname=binname,
                                    pi=pi,
                                    delta=delta,
                                    wordsize=wordsize,
                                    limbsizes=limbbits,
                                    method=method,
                                    primetype=prime_type,
                                    primename=f"{pi}_{delta}",
                                    blocksize=current_config.blocksize,
                                    keysize=current_config.keysize,
                                    iterations=settings.numtests,
                                ),
                                PFTestArith(
                                    name="test_add",
                                    binname=binname,
                                    pi=pi,
                                    delta=delta,
                                    wordsize=wordsize,
                                    limbsizes=limbbits,
                                    method=method,
                                    primetype=prime_type,
                                    primename=f"{pi}_{delta}",
                                    blocksize=current_config.blocksize,
                                    keysize=current_config.keysize,
                                    iterations=settings.numtests,
                                ),
                                PFTestArith(
                                    name="test_add_mix",
                                    binname=binname,
                                    pi=pi,
                                    delta=delta,
                                    wordsize=wordsize,
                                    limbsizes=limbbits,
                                    method=method,
                                    primetype=prime_type,
                                    primename=f"{pi}_{delta}",
                                    blocksize=current_config.blocksize,
                                    keysize=current_config.keysize,
                                    iterations=settings.numtests,
                                ),
                                PFTestArith(
                                    name="test_sqr",
                                    binname=binname,
                                    pi=pi,
                                    delta=delta,
                                    wordsize=wordsize,
                                    limbsizes=limbbits,
                                    method=method,
                                    primetype=prime_type,
                                    primename=f"{pi}_{delta}",
                                    blocksize=current_config.blocksize,
                                    keysize=current_config.keysize,
                                    iterations=settings.numtests,
                                ),
                                PFTestArith(
                                    name="test_carry_round",
                                    binname=binname,
                                    pi=pi,
                                    delta=delta,
                                    wordsize=wordsize,
                                    limbsizes=limbbits,
                                    method=method,
                                    primetype=prime_type,
                                    primename=f"{pi}_{delta}",
                                    blocksize=current_config.blocksize,
                                    keysize=current_config.keysize,
                                    iterations=settings.numtests,
                                ),
                                PFTestArith(
                                    name="test_sqr_no_carry",
                                    binname=binname,
                                    pi=pi,
                                    delta=delta,
                                    wordsize=wordsize,
                                    limbsizes=limbbits,
                                    method=method,
                                    primetype=prime_type,
                                    primename=f"{pi}_{delta}",
                                    blocksize=current_config.blocksize,
                                    keysize=current_config.keysize,
                                    iterations=settings.numtests,
                                ),
                                PFTestArith(
                                    name="test_add_dbl",
                                    binname=binname,
                                    pi=pi,
                                    delta=delta,
                                    wordsize=wordsize,
                                    limbsizes=limbbits,
                                    method=method,
                                    primetype=prime_type,
                                    primename=f"{pi}_{delta}",
                                    blocksize=current_config.blocksize,
                                    keysize=current_config.keysize,
                                    iterations=settings.numtests,
                                ),
                                PFTestArith(
                                    name="test_mul_no_carry",
                                    binname=binname,
                                    pi=pi,
                                    delta=delta,
                                    wordsize=wordsize,
                                    limbsizes=limbbits,
                                    method=method,
                                    primetype=prime_type,
                                    primename=f"{pi}_{delta}",
                                    blocksize=current_config.blocksize,
                                    keysize=current_config.keysize,
                                    iterations=settings.numtests,
                                ),
                                PFTestArith(
                                    name="test_reduce",
                                    binname=binname,
                                    pi=pi,
                                    delta=delta,
                                    wordsize=wordsize,
                                    limbsizes=limbbits,
                                    method=method,
                                    primetype=prime_type,
                                    primename=f"{pi}_{delta}",
                                    blocksize=current_config.blocksize,
                                    keysize=current_config.keysize,
                                    iterations=settings.numtests,
                                ),
                            ]
                        )
                        if current_config.multiplication.option == "precompute":
                            arithmetic_TestSuite.addTests(
                                [
                                    PFTestArith(
                                        name="test_mul_precomputed",
                                        binname=binname,
                                        pi=pi,
                                        delta=delta,
                                        wordsize=wordsize,
                                        limbsizes=limbbits,
                                        method=method,
                                        primetype=prime_type,
                                        primename=f"{pi}_{delta}",
                                        blocksize=current_config.blocksize,
                                        keysize=current_config.keysize,
                                        iterations=settings.numtests,
                                    ),
                                    PFTestArith(
                                        name="test_mul_precomputed_no_carry",
                                        binname=binname,
                                        pi=pi,
                                        delta=delta,
                                        wordsize=wordsize,
                                        limbsizes=limbbits,
                                        method=method,
                                        primetype=prime_type,
                                        primename=f"{pi}_{delta}",
                                        blocksize=current_config.blocksize,
                                        keysize=current_config.keysize,
                                        iterations=settings.numtests,
                                    ),
                                    PFTestArith(
                                        name="test_sqr_precomputed",
                                        binname=binname,
                                        pi=pi,
                                        delta=delta,
                                        wordsize=wordsize,
                                        limbsizes=limbbits,
                                        method=method,
                                        primetype=prime_type,
                                        primename=f"{pi}_{delta}",
                                        blocksize=current_config.blocksize,
                                        keysize=current_config.keysize,
                                        iterations=settings.numtests,
                                    ),
                                    PFTestArith(
                                        name="test_sqr_precomputed_no_carry",
                                        binname=binname,
                                        pi=pi,
                                        delta=delta,
                                        wordsize=wordsize,
                                        limbsizes=limbbits,
                                        method=method,
                                        primetype=prime_type,
                                        primename=f"{pi}_{delta}",
                                        blocksize=current_config.blocksize,
                                        keysize=current_config.keysize,
                                        iterations=settings.numtests,
                                    ),
                                ]
                            )
                    if settings.sage:
                        hash_TestSuite.addTest(
                            ClassicalPolynomial(
                                name="test_classical_polynomial",
                                binname=binname,
                                blocksize=current_config.blocksize,
                                keysize=current_config.keysize,
                                tagsize=current_config.tagsize,
                                pi=pi,
                                delta=delta,
                                transform=message_transform,
                                key_transform=key_transform,
                                numtests=settings.numtests,
                                full_logs=settings.full_logs,
                            )
                        )
                elif is_MersennePrimeFieldSpec(field):
                    prime_type: str = "1"
                    pi: int = field.pi
                    if current_config.blocksize * 8 + encodingMSB.bit_length() > pi:
                        warn(red("Encoding incompatible with chosen field size"))
                        exit(-1)
                    buffsize = ceil(pi / wordsize) * 8
                    macro_defs.append(f"-DBUFFSIZE={buffsize}")
                    print("generating Field Arithmetic")
                    with open(
                        f"src/field_arithmetic/mersenne_arithmetic_{prime_type}_"
                        + f"{pi}_"
                        + "_".join(map(str, current_config.limbs))
                        + f"_{current_config.wordsize}_{method}.h",
                        "w",
                        encoding="utf-8",
                    ) as outfile:
                        arithGen = MersenneArithmeticGenerator(
                            pi,
                            limbbits,
                            num_limbs,
                            wordsize,
                            buffsize,
                            blocksize=current_config.blocksize,
                            keysize=current_config.keysize,
                            file=outfile,
                            encodingMSB=encodingMSB,
                            lowerEncode=lowerEncode,
                            lastOnlyEnc=lastOnlyEnc,
                            encodingMask=encodingMask,
                            explicitEncoding=explicitEncoding,
                            nocheck=not settings.check_overflow,
                            doublecarry="doublecarry" in multiplication_options,
                            doublecarryover="doublecarryover" in multiplication_options,
                            doublecarry_temp="doublecarrytemp"
                            in multiplication_options,
                        )
                        arithGen.print_fieldmul()
                    make_cmd.append(f"PRIMETYPE={prime_type} PI={pi} PRIMENAME={pi}")
                    make_cmd.append("mersenne_arithmetic")
                    if settings.test_arith:
                        arithmetic_TestSuite.addTests(
                            [
                                PFTestArith(
                                    name="test_pack",
                                    binname=binname,
                                    pi=pi,
                                    delta=1,
                                    wordsize=wordsize,
                                    limbsizes=limbbits,
                                    method=method,
                                    primetype=prime_type,
                                    primename=f"{pi}",
                                    blocksize=current_config.blocksize,
                                    keysize=current_config.keysize,
                                    iterations=settings.numtests,
                                ),
                                PFTestArith(
                                    name="test_unpack_key",
                                    binname=binname,
                                    pi=pi,
                                    delta=1,
                                    wordsize=wordsize,
                                    limbsizes=limbbits,
                                    method=method,
                                    primetype=prime_type,
                                    primename=f"{pi}",
                                    blocksize=current_config.blocksize,
                                    keysize=current_config.keysize,
                                    iterations=settings.numtests,
                                ),
                                PFTestArith(
                                    name="test_unpack_msg",
                                    binname=binname,
                                    pi=pi,
                                    delta=1,
                                    wordsize=wordsize,
                                    limbsizes=limbbits,
                                    method=method,
                                    primetype=prime_type,
                                    primename=f"{pi}",
                                    blocksize=current_config.blocksize,
                                    keysize=current_config.keysize,
                                    iterations=settings.numtests,
                                ),
                                PFTestArith(
                                    name="test_mul",
                                    binname=binname,
                                    pi=pi,
                                    delta=1,
                                    wordsize=wordsize,
                                    limbsizes=limbbits,
                                    method=method,
                                    primetype=prime_type,
                                    primename=f"{pi}",
                                    blocksize=current_config.blocksize,
                                    keysize=current_config.keysize,
                                    iterations=settings.numtests,
                                ),
                                PFTestArith(
                                    name="test_add",
                                    binname=binname,
                                    pi=pi,
                                    delta=1,
                                    wordsize=wordsize,
                                    limbsizes=limbbits,
                                    method=method,
                                    primetype=prime_type,
                                    primename=f"{pi}",
                                    blocksize=current_config.blocksize,
                                    keysize=current_config.keysize,
                                    iterations=settings.numtests,
                                ),
                                PFTestArith(
                                    name="test_add_mix",
                                    binname=binname,
                                    pi=pi,
                                    delta=1,
                                    wordsize=wordsize,
                                    limbsizes=limbbits,
                                    method=method,
                                    primetype=prime_type,
                                    primename=f"{pi}",
                                    blocksize=current_config.blocksize,
                                    keysize=current_config.keysize,
                                    iterations=settings.numtests,
                                ),
                                PFTestArith(
                                    name="test_sqr",
                                    binname=binname,
                                    pi=pi,
                                    delta=1,
                                    wordsize=wordsize,
                                    limbsizes=limbbits,
                                    method=method,
                                    primetype=prime_type,
                                    primename=f"{pi}",
                                    blocksize=current_config.blocksize,
                                    keysize=current_config.keysize,
                                    iterations=settings.numtests,
                                ),
                                PFTestArith(
                                    name="test_carry_round",
                                    binname=binname,
                                    pi=pi,
                                    delta=1,
                                    wordsize=wordsize,
                                    limbsizes=limbbits,
                                    method=method,
                                    primetype=prime_type,
                                    primename=f"{pi}",
                                    blocksize=current_config.blocksize,
                                    keysize=current_config.keysize,
                                    iterations=settings.numtests,
                                ),
                                PFTestArith(
                                    name="test_sqr_no_carry",
                                    binname=binname,
                                    pi=pi,
                                    delta=1,
                                    wordsize=wordsize,
                                    limbsizes=limbbits,
                                    method=method,
                                    primetype=prime_type,
                                    primename=f"{pi}",
                                    blocksize=current_config.blocksize,
                                    keysize=current_config.keysize,
                                    iterations=settings.numtests,
                                ),
                                PFTestArith(
                                    name="test_add_dbl",
                                    binname=binname,
                                    pi=pi,
                                    delta=1,
                                    wordsize=wordsize,
                                    limbsizes=limbbits,
                                    method=method,
                                    primetype=prime_type,
                                    primename=f"{pi}",
                                    blocksize=current_config.blocksize,
                                    keysize=current_config.keysize,
                                    iterations=settings.numtests,
                                ),
                                PFTestArith(
                                    name="test_mul_no_carry",
                                    binname=binname,
                                    pi=pi,
                                    delta=1,
                                    wordsize=wordsize,
                                    limbsizes=limbbits,
                                    method=method,
                                    primetype=prime_type,
                                    primename=f"{pi}",
                                    blocksize=current_config.blocksize,
                                    keysize=current_config.keysize,
                                    iterations=settings.numtests,
                                ),
                                PFTestArith(
                                    name="test_reduce",
                                    binname=binname,
                                    pi=pi,
                                    delta=1,
                                    wordsize=wordsize,
                                    limbsizes=limbbits,
                                    method=method,
                                    primetype=prime_type,
                                    primename=f"{pi}",
                                    blocksize=current_config.blocksize,
                                    keysize=current_config.keysize,
                                    iterations=settings.numtests,
                                ),
                            ]
                        )
                    if settings.sage:
                        hash_TestSuite.addTest(
                            ClassicalPolynomial(
                                name="test_classical_polynomial",
                                binname=binname,
                                blocksize=current_config.blocksize,
                                keysize=current_config.keysize,
                                tagsize=current_config.tagsize,
                                pi=pi,
                                delta=1,
                                transform=message_transform,
                                key_transform=key_transform,
                                full_logs=settings.full_logs,
                            )
                        )
                else:
                    sys.exit(-1)
                make_cmd.append(f"LIMBBITS={'_'.join(map(str, current_config.limbs))}")
                make_cmd.append(f"WORDSIZE={current_config.wordsize}")
                make_cmd.append(f"METHOD={current_config.multiplication.method}")
                make_cmd.append(f'MDEFS="{" ".join(macro_defs)}"')
                make_cmd.append(f"BINNAME={binname}")
                make_cmd.append(f"BENCHRESNAME={binname}")
            elif is_BinaryFieldSpec(field):
                cpu_info = get_cpu_info()
                if cpu_info["arch"] in ["X86_32", "X86_64"]:
                    if "pclmulqdq" in cpu_info["flags"]:
                        ccflag = "-mpclmul"
                field_size: int = field.size
                polynomial: list[int] = polynomials[field_size]
                macro_defs.append(f"-DBUFFSIZE={wordsize}")
                if current_config.blocksize * 8 + encodingMSB.bit_length() > field_size:
                    warn(red("Encoding incompatible with chosen field size"))
                    exit(-1)
                print("generating Field Arithmetic")
                with open(
                    f"src/field_arithmetic/bf_arithmetic_{field.size}_"
                    + "_".join(map(str, current_config.limbs))
                    + f"_{current_config.wordsize}_{method}.h",
                    "w",
                    encoding="utf-8",
                ) as outfile:
                    arithGen = BinaryFieldArithmeticGenerator(
                        polynomial,
                        limbbits,
                        num_limbs,
                        wordsize,
                        file=outfile,
                        blocksize=current_config.blocksize,
                        keysize=current_config.keysize,
                        encodingMSB=encodingMSB,
                        lowerEncode=lowerEncode,
                        lastOnlyEnc=lastOnlyEnc,
                        encodingMask=encodingMask,
                        explicitEncoding=explicitEncoding,
                    )
                    arithGen.print_fieldmul()
                make_cmd.append("bf_arithmetic")
                # if '--no_test' not in options:
                if settings.test_arith:
                    arithmetic_TestSuite.addTests(
                        [
                            BFTestArith(
                                name="test_add",
                                binname=binname,
                                wordsize=wordsize,
                                limbsizes=limbbits,
                                method=method,
                                fieldsize=field_size,
                                coeffs=polynomial,
                                blocksize=current_config.blocksize,
                                keysize=current_config.keysize,
                                iterations=settings.numtests,
                            ),
                            BFTestArith(
                                name="test_add_dbl",
                                binname=binname,
                                wordsize=wordsize,
                                limbsizes=limbbits,
                                method=method,
                                fieldsize=field_size,
                                coeffs=polynomial,
                                blocksize=current_config.blocksize,
                                keysize=current_config.keysize,
                                iterations=settings.numtests,
                            ),
                            BFTestArith(
                                name="test_mul",
                                binname=binname,
                                wordsize=wordsize,
                                limbsizes=limbbits,
                                method=method,
                                fieldsize=field_size,
                                coeffs=polynomial,
                                blocksize=current_config.blocksize,
                                keysize=current_config.keysize,
                                iterations=settings.numtests,
                            ),
                            BFTestArith(
                                name="test_carry_round",
                                binname=binname,
                                wordsize=wordsize,
                                limbsizes=limbbits,
                                method=method,
                                fieldsize=field_size,
                                coeffs=polynomial,
                                blocksize=current_config.blocksize,
                                keysize=current_config.keysize,
                                iterations=settings.numtests,
                            ),
                            BFTestArith(
                                name="test_mul_no_carry",
                                binname=binname,
                                wordsize=wordsize,
                                limbsizes=limbbits,
                                method=method,
                                fieldsize=field_size,
                                coeffs=polynomial,
                                blocksize=current_config.blocksize,
                                keysize=current_config.keysize,
                                iterations=settings.numtests,
                            ),
                            BFTestArith(
                                name="test_sqr",
                                binname=binname,
                                wordsize=wordsize,
                                limbsizes=limbbits,
                                method=method,
                                fieldsize=field_size,
                                coeffs=polynomial,
                                blocksize=current_config.blocksize,
                                keysize=current_config.keysize,
                                iterations=settings.numtests,
                            ),
                            BFTestArith(
                                name="test_sqr_no_carry",
                                binname=binname,
                                wordsize=wordsize,
                                limbsizes=limbbits,
                                method=method,
                                fieldsize=field_size,
                                coeffs=polynomial,
                                blocksize=current_config.blocksize,
                                keysize=current_config.keysize,
                                iterations=settings.numtests,
                            ),
                        ]
                    )
                make_cmd.append(f"FIELDSIZE={field_size}")
                make_cmd.append(f"LIMBBITS={'_'.join(map(str, current_config.limbs))}")
                make_cmd.append(f"WORDSIZE={current_config.wordsize}")
                make_cmd.append(f"METHOD={current_config.multiplication.method}")
                make_cmd.append(f'MDEFS="{" ".join(macro_defs)}"')
                make_cmd.append(f"BINNAME={binname}")
                make_cmd.append(f"BENCHRESNAME={binname}")
            else:
                # Error this should not happen
                pass
        if settings.debug:
            make_cmd.append(f'CCFLAGS="-Og -ggdb {ccflag}"')
        else:
            make_cmd.append(f'CCFLAGS="-O3 {ccflag}"')
        if settings.verbose:
            make_cmd.append("VERBOSE=1")
        if settings.build:
            make_cmd.append("dir")
            if not ref:
                make_cmd.append("build")
                make_cmd.append("build_bench")
                if is_NewHashConfig(current_config) and is_BinaryFieldSpec(
                    current_config.field
                ):
                    make_cmd.append("build_binary_arith_test")
                else:
                    make_cmd.append("build_arith_test")
                make_cmd.append("build_lib")
                if settings.ctgrind:
                    make_cmd.append("build_ctgrind")
            else:
                make_cmd.append("build_reference")
            if os.system("which clang-format > /dev/null") == 0:
                make_cmd.append("pretty_print_intermediary")
            print("starting Build")
        if settings.verbose:
            print(" ".join(make_cmd))
        failure = os.system(" ".join(make_cmd)) != 0

        if settings.ctgrind:
            print("running ctgrind")
            res = subprocess.run(
                [
                    f"{settings.ctgrind_bin}",
                    "--leak-check=full",
                    "--track-origins=yes",
                    "--error-exitcode=42",
                    f"./bin/{binname}_ctgrind",
                ],
                capture_output=True,
                check=False,
            )
            if res.returncode:
                with open(f"results/{binname}_ctgrind.log", "bw") as logfile:
                    logfile.write(res.stderr)
            ctgrind_results[f"{file.name}_{config_number}"] = (res, current_config.name)
        if settings.test:
            print("running Tests:")
            results_path = f"results/{file.name}_{config_number}_test_results"
            with open(results_path, "w") as results_file:
                if settings.test_arith:
                    print("- Field Arithmetic Tests")
                    arith_res = unittest.TextTestRunner(
                        verbosity=2, stream=results_file
                    ).run(arithmetic_TestSuite)
                    arithmetic_test_results[f"{file.name}_{config_number}"] = (
                        arith_res,
                        current_config.name,
                    )
            with open(results_path, "a") as results_file:
                print("- Hash Function Tests")
                hash_res = unittest.TextTestRunner(
                    verbosity=2, stream=results_file
                ).run(hash_TestSuite)
                hash_test_results[f"{file.name}_{config_number}"] = (
                    hash_res,
                    current_config.name,
                )
            failure = not (
                hash_res.wasSuccessful()
                and ((not settings.test_arith) or arith_res.wasSuccessful())
            )
        if failure:
            if settings.bench:
                print(yellow("Skipping benchmark due to failure during tests or build"))
            linenums.pop()
            labels.pop()
        else:
            if settings.bench:
                print("starting benchmark")
                failure = os.system(f"./bin/{binname}_bench") != 0
                if failure:
                    print(yellow("Skipping plot due to failure bench"))
                    linenums.pop()
                    labels.pop()
            if settings.plot and not settings.plot_compare_only and not failure:
                print("starting plot")
                filename: str = f"{benchdir}{file.name}_{config_number}_results.csv"
                pltrs.plot(
                    filename,
                    name=f"{file.name}_{config_number}_{current_config.name}",
                    show_plots=settings.show_plots,
                    title=settings.plot_titles,
                    latex=settings.latex,
                    maxsize=settings.max_message_size,
                )
    if settings.plot:
        print("starting comparison plot")
        if len(linenums) > 0:
            plt_title: bool | str = settings.plot_titles
            if settings.plot_titles and config.name:
                plt_title = config.name
            pltrs.plot_compare(
                linenums,
                config=file.name,
                labels=labels,
                benchdir=benchdir,
                show_plots=settings.show_plots,
                title=plt_title,
                latex=settings.latex,
                maxsize=settings.max_message_size,
            )
        else:
            print(yellow("Nothin to plot!"))

if settings.ctgrind:
    print("ctgrind:")
    for name, (res, label) in ctgrind_results.items():
        if res.returncode == 42:
            print(
                red(
                    f"{label} ({name}): ctgrind found issues. Please check results/{name}_ctgrind.log for details."
                )
            )
        else:
            print(green(f"{label} ({name}): No issues found."))

if settings.test:
    if settings.test_arith:
        print("Arithmetic Tests:")
        for name, (res, label) in arithmetic_test_results.items():
            if res.wasSuccessful():
                print(green(f'{label} ({name}): {"SUCCESS"}'))
            else:
                print(
                    red(
                        f"{label} ({name}): {len(res.failures)} TESTS FAILED! \
                        Please see results/{name}_test_results."
                    )
                )

    print("Hash Tests:")
    for name, (res, label) in hash_test_results.items():
        if res.wasSuccessful():
            print(green(f'{label} ({name}): {"SUCCESS"}'))
        else:
            print(
                red(
                    f"{label} ({name}): {len(res.failures)} TESTS FAILED! \
                    Please see results/{name}_test_results."
                )
            )
