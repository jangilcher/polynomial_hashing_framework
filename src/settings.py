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

from typing import Optional


class Settings:
    def __init__(
        self,
        build: bool = True,
        bench: bool = True,
        plot: bool = True,
        test: bool = True,
        verbose: bool = False,
        tune: bool = False,
        debug: bool = False,
        plot_compare_only: bool = False,
        plot_titles: bool = True,
        iterations: int = 15,
        max_message_size: int = 16384,
        stepsize: int = 200,
        includes: Optional[list[str]] = None,
        sage: bool = False,
        show_plots: bool = False,
        check_overflow: bool = True,
        test_arith: bool = True,
        convert_only: bool = False,
        convert: bool = False,
        ctgrind: bool = False,
        ctgrind_bin: str = "ct_valgrind_3_21_0",
        latex: bool = False,
        numtests: int = 1000,
        full_logs: bool = False,
    ) -> None:
        self.build: bool = build
        self.bench: bool = bench
        self.plot: bool = plot
        self.test: bool = test
        self.test_arith: bool = test_arith
        self.verbose: bool = verbose
        self.tune: bool = tune
        self.debug: bool = debug
        self.plot_compare_only = plot_compare_only
        self.plot_titles: bool = plot_titles
        self.iterations: int = iterations
        self.max_message_size: int = max_message_size
        self.stepsize: int = stepsize
        self.sage: bool = sage
        self.show_plots: bool = show_plots
        self.check_overflow: bool = check_overflow
        self.convert_only: bool = convert_only
        self.convert: bool = convert or convert_only
        self.ctgrind: bool = ctgrind
        self.ctgrind_bin: str = ctgrind_bin
        self.latex: bool = latex
        self.numtests: int = numtests
        self.full_logs: bool = full_logs
        if includes is None:
            self.includes: list[str] = []
        else:
            self.includes: list[str] = includes

    def __str__(self):
        res: str = f"build = {self.build}\n"
        res += f"bench = {self.bench}\n"
        res += f"plot = {self.plot}\n"
        res += f"test = {self.test}\n"
        res += f"test_arith = {self.test_arith}\n"
        res += f"verbose = {self.verbose}\n"
        res += f"tune = {self.tune}\n"
        res += f"debug = {self.debug}\n"
        res += f"plot_compare_only = {self.plot_compare_only}\n"
        res += f"plot_titles = {self.plot_titles}\n"
        res += f"iterations = {self.iterations}\n"
        res += f"max_message_size = {self.max_message_size}\n"
        res += f"stepsize = {self.stepsize}\n"
        res += f"sage = {self.sage}\n"
        res += f"show_plots = {self.show_plots}\n"
        res += f"check_overflow = {self.check_overflow}\n"
        res += f"includes = {self.includes}"
        res += f"latex = {self.latex}"
        res = f"{{{res}}}"
        return res

    @staticmethod
    def from_options(opts: list[tuple[str, str]]) -> "Settings":
        options = list(map(lambda x: x[0], opts))
        settings = Settings(
            build="--no_build" not in options,
            bench="--no_bench" not in options,
            plot="--no_plot" not in options,
            test="--no_test" not in options,
            verbose="--verbose" in options,
            tune="--tune" in options,
            debug="--debug" in options,
            plot_compare_only="--plot_compare_only" in options,
            plot_titles="--no_plot_titles" not in options,
            show_plots="--show_plots" in options,
            check_overflow="--check_overflow" in options,
            test_arith="--no_test_arith" not in options,
            convert_only="--convert_only" in options,
            convert="--convert" in options,
            latex="--latex" in options,
            full_logs="--full_logs" in options,
        )
        if "--iterations" in options:
            try:
                idx: int = options.index("--iterations")
                settings.iterations = int(opts[idx][1])
            except ValueError:
                print("--iterations should be an integer")
                exit(-1)
        if "--max_messagesize" in options:
            try:
                idx = options.index("--max_messagesize")
                settings.max_message_size = int(opts[idx][1])
            except ValueError:
                print("--max_messagesize should be an integer")
                exit(-1)
        if "--stepsize" in options:
            try:
                idx = options.index("--stepsize")
                settings.stepsize = int(opts[idx][1])
            except ValueError:
                print("--stepsize should be an integer")
                exit(-1)
        if "--include" in options:
            additional_includes = [opt[1] for opt in opts if opt[0] == "--include"]
            if any(map(lambda s: s[-2:] != ".c", additional_includes)):
                print("--include must be .c files")
                exit(-1)
            settings.includes = additional_includes
        if "--ctgrind" in options:
            settings.ctgrind = True
            if "--ctgrind_bin" in options:
                idx = options.index("--ctgrind_bin")
                settings.ctgrind_bin = opts[idx][1]
        if "--numtests" in options:
            try:
                idx = options.index("--numtests")
                settings.numtests = int(opts[idx][1])
            except ValueError:
                print("--numtests should be an integer")
                exit(-1)
        return settings
