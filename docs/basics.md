# Setup

## Basic Dependencies

We depend on sagemath and several python libraries.
To ensure all these requirements are met we recommend the following steps:

1. Install the latest version of sagemath with python 3.10 (or newer) e.g.
as outlined [here](https://doc.sagemath.org/html/en/installation/conda.html).
1. In the sage environment run: `python3 -m pip install -r requirements.txt`
1. The generated C code depends on libsodium's RNG. Download and install
libsodium as outlined [here](https://doc.libsodium.org/installation).
Make sure gcc is able to find `libsodium.so`.

## Reference Implementations
We currently support reference implementations from libsodium, openssl and HACL*.

1. For openssl, make sure openssl (at least version 3) is installed on your system.
1. For HACL*, run `setup_hacl.sh`. This will download and build the gcc HACL* distribution and provide the necessary header files to the framework.
HACL* will not be installed globally on your system.

@import "old_grammar.md"
