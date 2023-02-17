[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

# OmicsDS
**Experimental** Columnar or Row Based Storage and Retrieval of BAM/BED/Feature-based Omics files in C++.

# Building
## Library Prerequisites
* CMake >= 3.6 minimum, >= 3.12 recomended for enabling parallel builds of testing suite
* make
* autoconf
* C++ compiler (clang and gcc compilers are tested as part of this repo's build process)
* OpenSSL library, currently only the 1.1.1 series is supported
* CURL library, OpenSSL backend
* Zlib library
* libUUID library
* Git

## Running the Build Process
This library follows a standard build process. From the root of the project run the following commands to build the library.
```bash
mkdir build && cd build
cmake ..
make
make install
```

## Documentation
You will need the following prerequisites for building the library documentation:
* doxygen
* sphinx
* breathe

Then run the following commands to generate the documentation, it will be generated under the `docs/sphinx` folder in your build directory.
```bash
mkdir build && cd build
cmake .. -DBUILD_DOCS=TRUE
make docs
```

## Running Tests
In addition to the library prerequisites, you will also need Catch2 installed. We support both Catch2 v2 and v3 for testing. Note that testing is automatically enabled or disabled based on the presence of the Catch2 library during initial configuration with `cmake`, so if you have subsequently installed Catch2 you will need to rerun the configuration process. Once you have Catch2, you can build the test binaries and run the tests by running one of the following commands:
```bash
make test
cmake --build . --target test --parallel <n> # Option if you have cmake >= 3.12, this will enable parallel building of the testing binaries which can speed up runtimes
```

## Example Build
Our CI pipeline for this repository builds and tests the library on both Ubuntu and MacOS. The pipelines can be a helpful reference for troubleshooting your own build process. The files are avaiable in this repository at `.github/workflows/basic.yml`.

## Bindings

### R
We provide an R package for interfacing with the C++ library. You will need all of the library prerequisites, in addition to an R installation. In order to build the R package, you can run the following commands
```bash
cmake .. -DR_LIBS=<location where R libraries should be installed defaults to ~/R-libs>
make omicsds_r_install
```

### Python
We provide a Python package for interfacing with the C++ library. You will need all of the library prerequisites, in addition to a Python installation. It is recommended to use an isolated environment for installing, for example with `venv`.
```bash
python -m venv .env
source .env/bin/activate
make omicsds_python
```

# Usage
## Library
The CLI is the currently the primary reference example for consumers of the library. The source files located at `tools/main` cover all of the major library features, with each file named to indicate which feature it demonstrates. The library documentation is also a helpful reference, instructions for generating it can be found in the [Documentation](#documentation) section.

## CLI
The CLI can be invoked directly to access its help text. Running the CLI with no arguments `omicsds` will print out the avaiable commands. Running the CLI with a command but no other text will print out a help message for that particular command `omicsds <command>`.

# Contributing
This section is intended for developers looking to contribute to the OmicsDS library.

## Development Docker
The `Dockerfile.developer` file can be used to quickly setup a build environment with the necessary build tools. If you have `docker` installed run:
```bash
docker build -f Dockerfile.developer -t omicsds:develop .
docker run -v <path to your OmicsDS repo>:/home/omicsds/OmicsDS -it omicsds:develop
```
This will mount your OmicsDS repository into the Docker and you can begin development.

## Branching
All your development should be done on a feature branch that has been branched off of `develop`. Once your feature is complete, open a PR against `develop` to have your changes incorporated.

## Adding Sources
This project makes use of `CMake` for managing our build process. The main modifications you will need to make to the `CMake` files are when adding new source files. The primary file for adding sources to the library is located at `src/main/CMakeLists.txt`. This defines a few `CMake` variables for the sources and includes that make up the library, and you can include your new files by adding them here. If you need to add new files to the CLI, its primary `CMake` file is at `tools/main/CMakeLists.txt`. Finally, the file at `src/resources/CMAKELists.txt` controls ProtoBuf generation which you will need to modify if you add any new `.proto` files to the library.

## Formatting
We use `clang-format` for formatting the C++ source code in this repository. We use a slightly modified version of the Google C++ code style guide, notably we use 100 characters for line lengths. There is a `.clang-format` configuration file in the root of this repository with the expected settings. You may find it helpful to have your editor automatically format your files using this `.clang-format` file, please consult your editor documentation for further details.

All your contributed code must be formatted correctly before it will be accepted. As part of building the project, a GitHub action will run to check that all code conforms to our style guide. If you would like to check if your code will be accepted by this step before pushing, you can run the script at `.github/scripts/clang_format.sh`.

## Testing
We have two main testing pathways in this repository.

### Source Tests
Both the library and CLI have `Catch2` tests for checking code functionality. These tests can be found at `src/test/cpp` for the library and `tools/test/cpp` for the CLI. If you contribute new source code, please add tests to these locations as appropriate to ensure we maintain good test coverage. The existing test cases can serve as an example for additional tests you would like to add.

If you need to add a new source file to the tests follow these steps:
1. Create your new test file in the appropriate `cpp` directory
2. Include `catch.h` in this file, this is a special header which allows us to run our test suite under both `Catch2` v2 and v3
3. Modify the `CMakeLists.txt` file in the `cpp` directory to include your new test file in the test sources

### Tool Tests
At the moment the only tool generated and tested is the `omicsds` CLI. This tool is exercised by a shell script located at `tools/test/test_cli.sh`. Invoking this test script manually is not recommended as it requires being able to find the paths to the test input files, instead invoke it through the `make tests` target. To expand the tests in this script, follow one of the existing test cases as an example no changes to the build will be needed to pick it up.
