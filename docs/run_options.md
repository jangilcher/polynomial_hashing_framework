# Guide to `run.py` Options

This document explains the commonly used options for `run.py`, the script for building, testing, benchmarking, and plotting polynomial hash configurations.

For each selected configuration, `run.py` performs the following high-level steps:
1. parse the configuration file;
2. generate field-arithmetic code for new hash configurations;
3. build the requested implementation and benchmark binaries;
4. run arithmetic and/or hash correctness tests, unless disabled;
5. optionally run ctgrind;
6. run benchmarks, unless disabled;
7. generate per-implementation plots and comparison plots, unless disabled.

## Basic usage

From the repository root:

```bash
python3 ./run.py [options] /path/to/config
```

You may pass one or more configuration files.
See `docs/new_grammar.md` and `docs/old_grammar.md` for the supported configuration formats.

Example:

```bash
python3 ./run.py \
  --stepsize=100 \
  --tune \
  --iterations=25 \
  --max_messagesize=16000 \
  --plot_y_cut=3 \
  --no_test \
  ./configs/multivariate_polynomials/config_fig4a.json
```


## Stage-control options

### `--no_build`

Skip the build stage.

By default, `run.py` invokes `make` to build the generated implementation, benchmark binary, arithmetic test library, hash shared library, and any required reference implementation targets.

### `--no_test`

Skip correctness tests.

By default, tests are enabled. When this flag is present, `run.py` does not run the arithmetic or hash test suites before benchmarking.
This is useful for faster benchmark-only runs.

### `--no_bench`

Skip benchmarking.

This is useful when you only want to build and/or test configurations.

### `--no_plot`

Skip plot generation.

This is useful for runs where you only want to build, test, or collect raw benchmark data.


## Benchmarking options

### `--stepsize=<bytes>`

Distance, in bytes, between benchmarked message sizes.

A smaller step size gives finer-grained benchmark curves but increases runtime. A larger step size reduces runtime but produces coarser plots.

### `--iterations=<n>`

Number of benchmark samples collected per message size.

Higher values generally produce more stable measurements but increase runtime.

### `--max_messagesize=<bytes>`

Largest message size, in bytes, to benchmark.

Together with `--stepsize`, this determines how many message sizes are benchmarked.

### `--bench_dir=<path>`

Set the base directory for benchmark output.


## Plotting options

### `--plot_y_cut=<value>`

Generate an additional plot with the y-axis capped at the specified value.

This is useful when slower implementations or outliers make the default plot difficult to read.

### `--plot_dir=<path>`

Set the base directory for generated plots.

### `--plot_compare_only`

Generate only comparison plots.

### `--no_plot_titles`

Disable plot titles.

### `--show_plots`

Display plots interactively in addition to writing them to disk.

### `--latex`

Enable LaTeX-style plotting output.

### `--fontsize=<n>`

Set the font size used in generated plots.

## Build and compiler options

### `--tune`

Compile with native tuning enabled.

When this option is set, `run.py` adds `-mtune=native` to the compiler flags. 

### `--debug`

Build with debug-oriented compiler flags and run hash tests in debug mode.

When enabled, `run.py` uses debug compiler flags such as `-Og -ggdb` instead of the normal optimized build flags. It also enables additional deterministic test cases in the hash-test harness.

### `--verbose`

Print additional build information.

This option passes verbose mode to the Makefile and prints the generated `make` command.


## Test options

### `--check_overflow`

Enable overflow checks in generated field-arithmetic code.

### `--no_test_arith`

Skip field-arithmetic tests.

When tests are enabled, `run.py` normally runs arithmetic tests for generated prime-field or binary-field arithmetic libraries. This option disables that arithmetic-test phase. Use this only when arithmetic has already been validated.

### `--no_test_hash`

Skip hash-function tests.

Hash tests compare the generated C implementation against the Sage reference model. This option disables hash-level correctness tests while leaving arithmetic tests enabled.

### `--numtests=<n>`

Set the number of randomized test cases used by the test harnesses.

Larger values increase test coverage but also increase runtime.

### `--test_steps=<n>`

Set the message-size step used by hash correctness tests.

This controls how densely the test harness samples message sizes across the tested range. Smaller values provide denser coverage but increase runtime.

### `--fail_fast`

Stop a test suite after the first failure.

This is useful while debugging a failing configuration because it reduces noise and avoids spending time on later test cases after the first error has been found.

### `--full_logs`

Print full messages and keys in test-failure output.

By default, long messages and keys may be abbreviated in failure messages. `--full_logs` is useful for debugging exact failing test vectors, but it can produce very large logs.

## ctgrind options

### `--ctgrind`

Build and run the ctgrind target.

When enabled, `run.py` builds the ctgrind binary and runs it under the configured ctgrind executable. If ctgrind reports an issue, the log is written under:

```text
results/<name>_ctgrind.log
```

### `--ctgrind_bin=<path-or-command>`

Set the ctgrind executable.

## Output files

Typical outputs are:

```text
bench/<timestamp>/      # Raw benchmark CSV files
plots/<timestamp>/      # Generated plots
results/<timestamp>/    # Additional generated results, when produced
```

When using Docker, these are usually mounted under:

```text
docker_data/bench/
docker_data/plots/
docker_data/results/
```
