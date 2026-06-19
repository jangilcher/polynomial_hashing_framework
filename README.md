# Polynomial Hashing Framework

This repository contains a code-generation, testing, and benchmarking framework for polynomial-based universal hash functions.

The framework originally accompanied the paper **"SoK: Efficient Design and Implementation of Polynomial Hash Functions over Prime Fields"**, published at the IEEE Symposium on Security and Privacy 2024. 
It has since been extended for **"New Designs of Multivariate-Polynomial Universal Hash Functions"** (to be published at ACM CCS 2026) to support multivariate polynomial hash designs, two-level hash constructions, and binary-field arithmetic.

The framework can be used to:
- generate C implementations of polynomial hash functions,
- test generated implementations against reference models,
- benchmark hash designs across message sizes and parameter choices,
- reproduce the performance experiments from the associated papers,
- compare new designs against reference implementations,
- experiment with new lower-level and higher-level polynomial constructions, for two-level polynomial hash designs.

## Repository structure

```text
.
├── configs/
│   ├── experiments/                       # Configurations for the IEEE S&P 2024 experiments
│   └── multivariate_polynomials/          # Configurations for the ACM CCS 2026 experiments (multivariate UHF)
├── docker_data/                           # Mounted output directory for Docker runs
├── docs/                                  # Additional documentation
├── ref/                                   # Reference implementations of comparison schemes
├── rijndael_aead/                         # Rijndael-based AEAD benchmark code
├── src/                                   # Framework source code
├── tests/                                 # Tests for correctness of C implementations against reference models
├── compose.yml                            # Docker Compose configuration
├── Dockerfile                             # Docker image definition
├── Makefile                               # Build rules used by the framework
├── requirements.txt                       # Python dependencies
├── run.py                                 # Main framework entry point
└── setup_hacl.sh                          # HACL* dependency setup script
```


## Setup
The easiest setup path is Docker. Manual setup is also supported.
### Docker Setup 
We provide the necessary docker compose files to easily setup all dependencies.
With docker compose properly setup on your system, simply run
```bash
docker compose build
```
to build the docker container.
The docker container will mount the subfolders in `./docker_data` as volumes to store outputs of the framework
(benchmarking data, plots etc.). To ensure that these are readable within the container run
```bash
chmod -R 777 docker_data/
```
once before the first launch of the container. 

To launch the container simply run:
```bash
docker compose run --rm hashing-framework
```

### Manual Setup
#### 1. Build and Install Libsodium
Follow the instructions here to build and install libsodium.
https://doc.libsodium.org/installation


#### 2. Install Sagemath (required for running the complete test suite, can be skipped if tests are not needed)
To install sagemath dependencies we recommend following the instructions here: https://doc.sagemath.org/html/en/installation/conda.html
In particular we recommend installing SageMath via
```bash
conda create -n sage_poly sage python=3.11
```
or
```bash
mamba create -n sage_poly sage python=3.11
```
(Note however, that depending on pre-existing software on the system one can run into issues with conda/mamba dependency resolution, in particular on arm based Macs.)

Finally activate the newly created `sage_poly` environment via `mamba activate sage_poly` or `conda activate sage_poly` and continue with the following commands in the same shell.

#### 3. Install framework dependencies
- Install python requirements as follows:
`python3 -m pip install -r requirements.txt`
- Setup hacl dependencies via `./setup_hacl.sh`

### Dependencies
This framework depends on python3.11, sagemath, and libsodium.
To be able to benchmark OpenSSL implementations we also depend on OpenSSL v3.


## Running a benchmark configuration
All commands should be run in the environment (docker or conda) where sagemath is installed!
We recommend deactivating all forms of dynamic frequency scaling before starting experiments or set the systems CPU to a fixed frequency.
On Linux systems this can be achieved using the `cpufrequtils` package (requires root).

To run a single configuration file (build and benchmark without tests) run the following command:
```bash
python3 ./run.py --stepsize=100 --tune --iterations=25 --max_messagesize=16000 --plot_y_cut=3 --no_test /path/to/config
```
This will compile, benchmark, and produce plots for the hash functions in the specified config.
Several examples of configuration files are stored under `configs/`.
Each invocation of the framework creates its own subfolder (named using a timestamp of time of invocation) in `bench` and `plots`
to store benchmarking data and plots, respectively.
The benchmark will measure 25 samples per messagesize upto 16000 B in steps of 100 B.
Depending on the system and number of configurations this can require a very long amount of time.
You can adjust these settings by changing the following parameters:
- `--stepsize` defines the distance in bytes between messagesizes.
- `--iterations` defines the number of samples per messagesize, it is not recommended to set this to values lower than 5.
- `--max_messagesize` defines the maximum message size to be benchmarked.

By default several plots are created so that all lines are visible.
This however can sometimes make reading graphs very hard.
The `--plot_y_cut` parameter causes an additional plot to be generated where the maximum y-axis value is user specified.

To also run tests simply remove the `--no_test` argument, be advised however that testing can take a large amount of time, especially on weaker hardware.

Additional options for running benchmarks are described in `docs/` as well as a guide to create/modify configuration files.

## Reproducing the experiments of "SoK: Efficient Design and Implementation of Polynomial Hash Functions over Prime Fields"

The configuration files used for the benchmarks in **"SoK: Efficient Design and Implementation of Polynomial Hash Functions over Prime Fields"** are located in:

```text
./configs/experiments/
```

Run them with `run.py` using the same command structure shown above.

## Reproducing the experiments of "New Designs of Multivariate-Polynomial Universal Hash Functions"

### Multivariate-Polynomial hash benchmarks:
The configuration files used for the benchmarks of multivariate-polynomial hash in **"New Designs of Multivariate-Polynomial Universal Hash Functions"** are located in:

```text
./configs/multivariate_polynomials/
```

#### Main binary-field performance results
To reproduce our main performance results for binary fields run 
```bash
python3 ./run.py --stepsize=100 --tune --iterations=25 --max_messagesize=16000 --plot_y_cut=1.8 --no_test ./configs/multivariate_polynomials/config_fig4a.json
python3 ./run.py --stepsize=100 --tune --iterations=25 --max_messagesize=16000 --plot_y_cut=1.8 --no_test ./configs/multivariate_polynomials/config_fig4b.json
```
This will create plots similar to Figure 4 in the ACM CCS version of the paper.
Framework runs produce benchmark data and plots in timestamped subdirectories.
Typical outputs include:

```text
bench/<timestamp>/      # Raw benchmark data
plots/<timestamp>/      # Generated plots
```

When using Docker, output directories are mounted through `docker_data/`.

#### d-2LHash reference configurations

The following configurations run the d-2LHash variants provided by Bhattacharyya et al.:

```text
./configs/multivariate_polynomials/config_reference_d2lhash1271.json
./configs/multivariate_polynomials/config_reference_d2lhash1305.json
```

#### Exploratory configurations
`./configs/multivariate_polynomials/experiments/` contains all configurations used for the design-space exploration phase.
(Be aware that running all of these can take weeks to months)

### Rijndael-based AEAD benchmarks:

The AEAD implementations are in the subfolder `rijndael_aead`. This includes implementations with different unroll factors.
To run the best version for each AEAD variant (i.e. the concrete implementation variants included in the paper), execute the following commands: 

```bash
cd rijndael_aead
make MODE=bench FOLDER=bench/ REPETITIONS=25 STEPSIZE=1 ITERATIONS=1000 all 
mkdir bench
./bin/rijndael256x6x_no_hash_bench
./bin/rijndael256x_mhp_nmh128x6_bench
./bin/rijndael256x_mhp_nmh256x6_bench
./bin/rijndael256x_poly128x4_bench
./bin/rijndael256x_poly256x4_bench
```

Once the benchmarks are done, plots can be created by running `plot.py` using python3 inside the `rijndael_aead` folder.
Note: The plotting depends on: numpy, scipy, pandas, and matplotlib. These requirements are already installed in the environment created for the framework above, so we recommend running it in the same environment.


-----------------------------------------------------------------------------------------------------
## Documentation

Additional documentation is available in `./docs/`:

```text
docs/ 
├── basics.md                               # Setup notes and external dependencies 
├── new_grammar.md                          # Current JSON format for configuration files
├── old_grammar.md                          # Legacy format for configuration files 
├── run_options.md                          # Command-line options for run.py 
├── adding_new_polynomial.md                # Guide to adding a new generated polynomial construction 
├── adding_new_reference_implementation.md  # Guide to adding a new external reference implementation 
└── extracting_generated_c_code.md          # Guide to extracting generated C code for a specific config
```

The documentation is still a work in progress.
It currently includes:
- a guide to configuration files (used to select and generate polynomial hash benchmarks),
- a guide on options to `run.py` (used to run configuration files),
- a guide to add new polynomials to the framework,
- a guide to add new reference implementations to the framework,
- a guide to extract the generated C code for a specific configuration, i.e., the code for the polynomial, the field arithmetic and the transforms.


## Licenses

Unless otherwise specified all code is licensed under the MIT License.
A notable exception to this is the boost preprocessor library which we rely on and distribute as part of this software.
The boost preprocessor library is licensed under the Boost license, please see `Boost_LICENSE_1_0.txt` for details.
For ease of reproducing results `./ref` contains various reference implementations of other schemes.
Please see the corresponding licenses in the corresponding files.
