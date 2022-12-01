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
