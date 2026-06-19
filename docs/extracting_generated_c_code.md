# Extracting Generated C Code for a Specific Configuration

This guide explains how to locate and extract the C code and binaries produced for a single configuration.

The framework is primarily a code-generation and benchmarking environment. 
A run of `run.py` builds a selected configuration by generating field-arithmetic code, selecting polynomial and transform implementations through preprocessor macros, invoking the Makefile, and writing outputs under `bin/`, `obj/`, `asm/`, `bench/`, and `plots/`.

For a framework-generated hash, the final implementation is assembled from several pieces:
- selected polynomial source files from `src/polynomial/`;
- selected transform source files from `src/transform/`;
- generated field-arithmetic headers under `src/field_arithmetic/`.

For most purposes, the easiest extractable artifact is the preprocessed C output in `asm/<binname>/*.i`, because it contains the macro-expanded version of the original C code.

## Generated and intermediate files

After the build, inspect:

```text
bin/
obj/<binname>/
asm/<binname>/
src/field_arithmetic/
```

Typical files are:

```text
bin/<binname>                    # standalone executable
bin/<binname>_bench              # benchmark binary

obj/<binname>/*.o                # compiled object files

asm/<binname>/*.s                # assembly output
asm/<binname>/*.i                # preprocessed C output

src/field_arithmetic/field_arithmetic.h
src/field_arithmetic/*arithmetic*.h
```

The file `src/field_arithmetic/field_arithmetic.h` is the active arithmetic header selected for the most recent build. The more specific generated arithmetic header also remains under `src/field_arithmetic/` with a name that encodes the field type, field size, limb layout, word size, and multiplication method.

### how to determine `<binname>`?

For a generated hash configuration, `run.py` uses as `<binname>`:

```text
<config-file-name>_<configuration-index>
```

where `<config-file-name>` is the name of the config file and `<configuration-index>` is the position of the configuration in the config file.
Example: `config_fig4a.json_3`.

`run.py` prints messages of the form:

```text
Working on config: <config-file>:<index>: <name>
```

Use this line to confirm the exact file name and index.

## Extract original C code for polynomials

The C code to extract for polynomials are:

```text
src/polynomial/<OUTERPOLY>.c
src/polynomial/<OUTERPOLY>.h
src/polynomial/<INNERPOLY>_inner.h   # if used
```

The `--verbose` output shows the exact Makefile variables used for `OUTERPOLY`, `OUTERPOLY_H`, `INNERPOLY`, and `INNERPOLY_H`.

The original C code for polynomials often uses macros to support different set of parameters. 
This can make the code harder to parse. 
A macro-expanded version of this code for a selected parameter set can be generated with `run.py` and will be saved in `asm/<binname>/*.i`.


## Extract preprocessed C code

The files in `asm/<binname>/*.i` are preprocessed C code. 
They are useful when you want to inspect the exact code seen by the compiler after includes and macros have been expanded.


## Extract assembly code

The files in `asm/<binname>/*.s` are the generated assembly files from the build. 
These are useful for checking whether compiler optimizations were applied.

