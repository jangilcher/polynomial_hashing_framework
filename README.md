# Polynomial Hashing Framework
This repo contains the polynomial hashing framework from "SoK: Efficient Design and Implementation of Polynomial Hash Functions over Prime Fields"
to appear at IEEE Symposium on Security and Privacy 2024.
The configurations for the experiments mentioned in that paper are in `./configs/experiments`.
Documentations (still work in progress) can be found in `./docs`.

# Dependencies
This framework depends on python3.11, sagemath, and libsodium.
To be able to benchmark OpenSSL implementations we also depend on OpenSSL v3.

# Licenses

Unless otherwise specified all code is licensed under the MIT License.
A notable exception to this is the boost preprocessor library which we rely on and distribute as part of this software.
The boost preprocessor library is licensed under the Boost license, please see `Boost_LICENSE_1_0.txt` for details.
