# Adding a New Reference Implementation

This guide explains how to add a new external reference implementation to the benchmarking framework.

Reference implementations are selected in JSON configuration files with:

```json
"ref": true
```

They are different from framework-generated polynomial hashes. A reference implementation is expected to provide a C wrapper with the same hash interface used by the benchmarking framework.

## Overview

Adding a new reference implementation requires:

1. add any external source code or headers under `ref/`;
2. add a wrapper under `src/bench/ref/`;
3. register the implementation in `src/reference_params.py`, 
and extend `src/config_spec.py` if the new `lib`, `mac`, or `implementation` value is not currently accepted;
4. update the Makefile to compile the required additional objects, libraries, etc.


## Selecting a reference implementation in a configuration file

A reference configuration has this shape:

```json
{
  "name": "MyLib MyHash (Display name used in plots)",
  "ref": true,
  "lib": "mylib",
  "mac": "myhash",
  "implementation": "myimplem",
  "skip": false,
  "description": ""
}
```
### Reference key
`run.py` builds a lookup key as:

```text
<lib>_<mac>
```

or, when `implementation` is not `null`:

```text
<lib>_<mac>_<implementation>
```

This key must exist in `src/reference_params.py`. 
These values become part of filenames and Makefile variables.

## Add a C wrapper file

Create a C wrapper file in:

```text
src/bench/ref/
```

The filename must follow this pattern:

```text
src/bench/ref/ref_<reference-key>.c
```

For example:
`src/bench/ref/ref_mylib_myhash_myimplem.c`

The Makefile compiles reference wrappers using the `ref_<reference-key>.c` naming convention, so the filename must match the key exactly.

### Implementation of the C wrapper

The wrapper should define the functions expected by the benchmarking framework.
A minimal wrapper has this shape:

```c
#include "hash.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * Include the external implementation's headers here.
 * Example:
 * #include "../../../ref/mylib/myhash.h"
 */

void init_hash(void) {
    /*
     * Optional one-time initialization.
     * Leave empty if the reference implementation does not need setup.
     */
}

void hash(
    unsigned char *out,
    const unsigned char *in,
    unsigned long long inlen,
    unsigned char *key,
    const unsigned long long keylen
) {
    /*
     * Call the reference implementation here.
     *
     * The parameters are:
     *   out    - tag output buffer of OUTPUTSIZE bytes
     *   in     - message buffer
     *   inlen  - message length in bytes
     *   key    - key buffer of KEYSIZE bytes
     *   keylen - key length in bytes
     */
}

int hash_verify(
    unsigned char *out,
    const unsigned char *in,
    unsigned long long inlen,
    const unsigned char *key
) {
    /*
     * Optional verification code.
     * Existing wrappers may leave this empty if unused.
     */
    return 0;
}
```

Keep the wrapper simple. It should adapt the external implementation to the framework's `hash` interface, not reimplement the algorithm.

## Link the reference implementation to the framework

### Adding parameters for the new reference implementation

Modify `src/reference_params.py` to set up the parameters (block, key, tag sizes, and flags) required for the new reference implementation.
To do so, add an entry for the reference key, for example:

```python
"mylib_myhash_myimplem": {
    "blocksize": 16,
    "keysize": 32,
    "outputsize": 16,
    "flags": "USE_MYLIB=1",
},
```

The fields are:

- `blocksize`: message block size in bytes;
- `keysize`: key size in bytes passed by the benchmarking framework;
- `outputsize`: tag size in bytes;
- `flags`: Makefile variables or preprocessor settings required to build the reference.

If no special flags are needed, use `"flags": ""`.

### Extending the framework to support the reference implementation

To link a reference implementation to the framework, update the file `src/config_spec.py`.

If the new `lib`, `mac`, or `implementation` value is not currently accepted, extend the relevant `Literal[...]` type in the `ReferenceConfig` class.

For example, if adding:

```json
"lib": "mylib",
"mac": "myhash",
"implementation": "myimplem"
```

then add `"mylib"` to the list `lib: Literal[]` and `"myhash"` to the list `mac: Literal[]` in the `ReferenceConfig` class so that the parser accepts them.


## Update the Makefile (if needed)

The default reference build compiles `src/bench/ref/ref_<reference-key>.c`
and links it with the benchmarking framework.

If the wrapper depends only on headers and system libraries already available to the build, this may be sufficient.
If the reference needs additional source files, assembly files, include directories, libraries, or compiler flags, update the Makefile.

Common changes include:

#### Additional include path

```make
override INCDIRS := $(INCDIRS) -Iref/mylib/include
```

#### Additional object dependency

```make
ifdef USE_MYLIB
override REFDEPS := $(REFDEPS) $(OBJDIR)/mylib.o
endif
```

#### Compile a source file from `ref/`

```make
$(ASMDIR)/mylib.s: ref/mylib/src/mylib.c
    $(CC) -S $(DEFS) $(CCFLAGS) -o $@ -c $< $(INCDIRS) $(LDFLAGS)

$(OBJDIR)/mylib.o: $(ASMDIR)/mylib.s
    $(CC) $(DEFS) $(CCFLAGS) -o $@ -c $< $(INCDIRS)
```

#### Link an additional library

```make
ifdef USE_MYLIB
override LDFLAGS := $(LDFLAGS) -lmylib
endif
```

Use a dedicated flag such as `USE_MYLIB=1` so the extra build logic applies only to the new reference implementation.

