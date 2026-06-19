# Adding a New Polynomial Construction

This guide explains how to add a new polynomial hash construction to the framework.

The framework selects polynomial implementations at build time from a JSON configuration file.
`run.py` converts these configuration values into Makefile variables such as `OUTERPOLY`, `OUTERPOLY_H`, `INNERPOLY`, and `INNERPOLY_H`, and the Makefile builds the corresponding objects from `src/polynomial/`.

## Overview

Adding a new polynomial requires:

1. add the C implementation under `src/polynomial/`;
2. add a Sage reference model under `tests/polynomial.sage`, and a test hook in `tests/test_polynomial.py`, if the new construction should be tested;
3. create a JSON configuration file that selects the new polynomial.

For quick experimental benchmarking, only the C implementation and a configuration are required. For correctness testing, the Sage model and Python test hook are also required.

## File layout

Polynomial implementations live in:

```text
src/polynomial/
```

A typical polynomial implementation has at least:

```text
src/polynomial/<name>.h
src/polynomial/<name>.c
```

A two-level construction may additionally define an inner-polynomial header:

```text
src/polynomial/<name>_inner.h
```

Existing examples include:

```text
// for the polynomial d-2LHash
src/polynomial/d2LHP.h
src/polynomial/d2LHP.c

// for the two-level polynomial tMMH-NMH
src/polynomial/tMMH.h
src/polynomial/tMMH.c
src/polynomial/NMH_NB_Delay_inner.h

// for the polynomial NMH 
src/polynomial/NMH_NB_Delay.h
src/polynomial/NMH_NB_Delay.c
src/polynomial/NMH_NB_Delay_inner.h
```

Use one of the existing constructions as a template. For a simple one-level construction, start from an implementation such as `d-2LHash`. 
For a two-level construction, start from `tMMH`.
In the above examples, NMH can be used as inner-polynomial in a two-level construction or as standalone polynomial.


## Selecting a polynomial in a configuration file

A minimal one-level configuration looks like this:

```json
{
  "name": "MyPoly test",
  "ref": false,
  "keysize": 16,
  "blocksize": 16,
  "tagsize": 16,
  "field": {
    "field_type": "binary",
    "size": 128
  },
  "wordsize": 64,
  "limbs": [64, 64],
  "multiplication": {
    "method": "schoolbook",
    "options": null
  },
  "key_transform": {
    "id": 1,
    "options": null
  },
  "msg_transform": {
    "id": 1,
    "options": null
  },
  "field_transform": {
    "id": 1
  },
  "hash_transform": {
    "name": "simple_key_reuse_length_encoding"
  },
  "polynomial": {
    "name": "MyPoly",
    "num_keys": 1,
    "parameters": [],
    "inner_polynomial": null,
    "test": { "name": "MyPoly" }
  },
  "keygenerator": {
    "required": false,
    "number_of_bytes": null
  },
  "description": ""
}
```

Place this object in a configuration file under `configs/`.

### Naming rules

The polynomial name in the configuration file must match the source file and function names.

For example, this configuration fragment:

```json
"polynomial": {
  "name": "MyPoly",
  "num_keys": 1,
  "parameters": [],
  "inner_polynomial": null,
  "test": { "name": "MyPoly" }
}
```

expects files and a function of the form:

```text
src/polynomial/MyPoly.h
src/polynomial/MyPoly.c
void MyPoly(...);
```

For a two-level construction, the inner polynomial name follows the same rule. The framework appends `_inner` when it wires an inner polynomial into the build:

```json
"inner_polynomial": {
  "polynomial": {
    "name": "MyInnerPoly",
    "parameters": [],
    "inner_polynomial": null,
    "test": { "name": "MyInnerPoly" }
  },
  "superblocksize": 32,
  "superkeysize": 32
}
```

This expects the relevant inner implementation to be available as:

```text
src/polynomial/MyInnerPoly_inner.h
```


## C implementation of a polynomial

The C header file should declare the polynomial function with this signature:

```c
#ifndef __MYPOLY_H
#define __MYPOLY_H

void MyPoly(
    unsigned char *out,
    const unsigned char *in,
    unsigned long long inlen,
    const unsigned char *key,
    unsigned long long keylen
);

#endif
```


A typical polynomial implementation includes:

```c
#define OUTER 1

#include "../field_arithmetic/field_arithmetic.h"
#include "../transform/transform.h"
#include <stddef.h>
#include <string.h>

#if EXPLICIT_LENGTH_ENCODE
#include "../length_encoding.h"
#endif

#include "MyPoly.h"

void MyPoly(
    unsigned char *out,
    const unsigned char *in,
    unsigned long long inlen,
    const unsigned char *key,
    unsigned long long keylen
) {

    /*
     * 1. Transform and unpack message/key material as needed.
     * 2. Evaluate the polynomial using the generated field arithmetic.
     * 3. Apply explicit length encoding when enabled.
     * 4. Pack and transform the field element into the output tag.
     */

}
```

### Field-arithmetic API

Polynomial implementations should use the generated field-arithmetic interface in:

```c
#include "../field_arithmetic/field_arithmetic.h"
```

Common operations are:

```c
unpack_field_elem(...)
pack_field_elem(...)
field_mul(...)
field_add(...)
```

The exact available operations depend on the generated arithmetic backend and whether the field is prime or binary.

### Key length and key generation

Polynomials that require key material that depends on the message length need to activate the option in the configuration file:

```json
"keygenerator": {
  "required": true,
  "number_of_bytes": null
}
```

Additionally, for such polynomials, define a function
```c
unsigned long long get_keylength(unsigned long long inlen)
```
in the polynomial C header, that computes the number of field-element keys required to process a message of length `inlen`.

A fixed-size key construction may not need generated key material. In that case, set:

```json
"keygenerator": {
  "required": false,
  "number_of_bytes": null
}
```

and make sure `polynomial.num_keys` is set correctly in the configuration file.

### Parameters

It is possible to use in the C code, parameters set in the config file.
Use parameters for construction choices such as delay factor, number of branches, etc.

In the config file, the JSON field:

```json
"parameters": [8, 1, 0]
```

is passed to the C build as preprocessor macros:

```c
OUTER_PARAM0
OUTER_PARAM1
OUTER_PARAM2
```

For inner polynomials, parameters are passed as:

```c
INNER_PARAM0
INNER_PARAM1
INNER_PARAM2
```


Parameters can be used in the C code as follows:

```c
#ifndef OUTER_PARAM0
#define OUTER_PARAM0 1
#endif

#define MY_DELAY OUTER_PARAM0
```

### Two-level constructions

For a two-level construction, the outer/higher-level polynomial processes the outputs of an inner/lower-level polynomial. 
An example of inner/lower-level polynomial specified in a JSON configuration is:

```json
"inner_polynomial": {
  "polynomial": {
    "name": "NMH_NB_Delay",
    "parameters": [8, 1, 0, 1],
    "inner_polynomial": null,
    "test": { "name": "NMH" }
  },
  "superblocksize": 24,
  "superkeysize": 24
}
```

The fields have the following meaning:

- `inner_polynomial.polynomial.name`: C implementation name for the inner/lower-level polynomial;
- `inner_polynomial.polynomial.parameters`: parameters exposed as `INNER_PARAM<i>`;
- `superblocksize`: number of message blocks processed by one inner/lower-level instance;
- `superkeysize`: number of key blocks used by one lower-level instance.

When `inner_polynomial` is present, `run.py` defines macros such as:

```c
NB_SUPERBLOCKS
NB_SUPERKEYS
SUPERBLOCKSIZE
SUPERKEYSIZE
```

and tells the Makefile which inner header to include.



## Adding correctness test

Correctness tests compare the generated C implementation against the Sage reference model in:

```text
tests/polynomial.sage
```

In the configuration file, the JSON `polynomial.test.name` field selects the Python test hook and Sage reference model. For example:

```json
"test": {
  "name": "MyPoly"
}
```

maps to a Python method named `test_MyPoly` in `tests/test_polynomial.py`,
and to a Sage class `MyPoly` in `tests/polynomial.sage`.


### Adding a Sage reference model

Add in `tests/polynomial.sage` a class for the new construction following the existing style. The reference model should use the same:

- field,
- block size,
- key size,
- tag size,
- message transform,
- key transform,
- hash transform,
- polynomial parameters.


### Adding the Python test hook

If the new test name is not already handled, add a method to the `Polynomial` test class in:

```text
tests/test_polynomial.py
```

The method should instantiate the Sage reference model and then call `_runTestBattery`.

Example pattern:

```python
def test_MyPoly(self) -> None:
    hash = mod.MyPoly(
        field=self.getField(),
        tagbytes=self.tagsize,
        keybytes=self.keysize,
        blockbytes=self.blocksize,
        transform=self.transform,
        key_transform=self.key_transform,
        hash_transform=self.hash_transform,
    )
    self._runTestBattery(hash, keyGen=False)
```

For a two-level construction, follow the pattern used by existing tests such as `test_tMMH`.

