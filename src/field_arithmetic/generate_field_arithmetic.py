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

#!/usr/bin/python

import sys
import getopt

from src.field_arithmetic.ArithmeticGenerator import ArithmeticGenerator
from src.field_arithmetic.MersenneArithmeticGenerator import MersenneArithmeticGenerator
from src.field_arithmetic.PrecomputingCrandallArithmeticGenerator import (
    PrecomputingCrandallArithmeticGenerator,
)
from src.field_arithmetic.CrandallArithmeticGenerator import CrandallArithmeticGenerator
from src.field_arithmetic.BinaryFieldArithmeticGenerator import (
    BinaryFieldArithmeticGenerator,
)


class bcolors:
    RED = "\x1B[31m"
    GRN = "\x1B[32m"
    YEL = "\x1B[33m"
    BLU = "\x1B[34m"
    MAG = "\x1B[35m"
    CYN = "\x1B[36m"
    WHT = "\x1B[37m"
    RESET = "\x1B[0m"


def main(argv):
    PI = 266
    DELTA = 3
    LIMBBITS = 54
    LIMBBITS2 = 50
    NUMLIMBS = 5
    WORDSIZE = 64
    DOUBLECARRYOVER = False
    opts, args = getopt.getopt(
        argv,
        "",
        [
            "pi=",
            "delta=",
            "limbbits=",
            "limbbits2=",
            "numlimbs=",
            "wordsize=",
            "doubleCarryOver=",
        ],
    )
    for (
        opt,
        arg,
    ) in opts:
        if opt == "--pi":
            PI = int(arg)
        elif opt == "--delta":
            DELTA = int(arg)
        elif opt == "--limbbits":
            LIMBBITS = int(arg)
        elif opt == "--limbbits2":
            LIMBBITS2 = int(arg)
        elif opt == "--numlimbs":
            NUMLIMBS = int(arg)
        elif opt == "--wordsize":
            WORDSIZE = int(arg)
        elif opt == "--doubleCarryOver":
            if arg == "True":
                DOUBLECARRYOVER = True
            elif arg == "False":
                DOUBLECARRYOVER = False
    # gen = CrandallArithmeticGenerator(
    #     PI, DELTA, [LIMBBITS]*(NUMLIMBS-1) + [LIMBBITS2], NUMLIMBS, WORDSIZE)
    gen = BinaryFieldArithmeticGenerator([64, 4, 3, 1, 0], 64, 3, 64)
    gen = BinaryFieldArithmeticGenerator([128, 7, 2, 1, 0], 64, 3, 64)
    gen = BinaryFieldArithmeticGenerator([192, 7, 2, 1, 0], 64, 3, 64)
    # gen = BinaryFieldArithmeticGenerator([256,10,5,2,0],64,3,64)
    gen.print_fieldmul(DOUBLECARRYOVER)


if __name__ == "__main__":
    main(sys.argv[1:])
