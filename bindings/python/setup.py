#
# setup.py
#
# The MIT License
#
# Copyright (c) 2023 Omics Data Automation, Inc.
# Copyright (c) 2023-2024 dātma, inc™
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
# Description: setup script to build the OmicsDS python package
#
import sys
import os
import numpy as np

from setuptools import setup, Extension, find_packages

# Try reading install location from environment
OMICSDS_INSTALL_PATH = os.getenv("OMICSDS_HOME", default="/usr/local")
WITH_VERSION = "0.0.1"
USE_CYTHON = False
EXT = "cpp"

# Scan environment and arg list to determine if we should cythonize or not
print(f"cwd={os.getcwd()} args: {sys.argv}")
for arg in sys.argv[:]:
    if arg.find("--with-omicsds=") == 0 and len(arg.split("=")[1]) > 0:
        OMICSDS_INSTALL_PATH = os.path.expanduser(arg.split("=")[1])
        sys.argv.remove(arg)
    if arg.find("--with-version=") == 0 and len(arg.split("=")[1]) > 0:
        WITH_VERSION = arg.split("=")[1]
        sys.argv.remove(arg)
    if arg.find("--cythonize") == 0:
        USE_CYTHON = True
        EXT = "pyx"
        sys.argv.remove(arg)

print(f"Using {OMICSDS_INSTALL_PATH} for OmicsDS library.")

OMICSDS_INCLUDE_DIR = os.path.join(OMICSDS_INSTALL_PATH, "include/omicsds")
OMICSDS_LIB_DIR = os.path.join(OMICSDS_INSTALL_PATH, "lib")

EXTENSIONS = {
    "api": {
        "language": "c++",
        "include_dirs": [
            OMICSDS_INCLUDE_DIR,
            "src/_processor",
            np.get_include(),
        ],
        "sources": [
            f"omicsds/api.{EXT}",
            "src/_processor/omicsds_processor.cc"
        ],
        "libraries": [
            "omicsds",
        ],
        "library_dirs": [
            OMICSDS_LIB_DIR,
        ],
        "extra_compile_args": [
            "-std=c++17",
        ],
    }
}

print(f"Using cython: {USE_CYTHON}")

def build_extensions():
    if USE_CYTHON:
        from Cython.Build.Dependencies import cythonize
    extensions = []
    for extension, config in EXTENSIONS.items():
        print(extension)
        extensions.append(
            Extension(
                name=f"omicsds.{extension}",
                **config,
            )
        )
    if USE_CYTHON:
        extensions = cythonize(extensions, force=True)
    return extensions

with open("requirements/prod.txt") as f:
    prod_requirements = f.readlines()

with open("requirements/dev.txt") as f:
    # skip over prod inclusion
    dev_requirements = f.readlines()[2:]

with open("README.md") as f:
    description = f.read()

setup(
    name="omicsds",
    description="Experimental Python Bindings for querying OmicsDS",
    long_description=description,
    long_description_content_type="text/markdown",
    author="datma inc.",
    author_email="support@datma.com",
    maintainer="datma inc.",
    maintainer_email="support@datma.com",
    license="MIT",
    zip_safe=False,
    ext_modules=build_extensions(),
    install_requires=prod_requirements,
    extras_require={
        "dev": dev_requirements,
    },
    python_requires=">=3.8",
    packages=find_packages(include=["omicsds", "omicsds.*"]),
    keywords=["genomics", "omicsds"],
    include_package_data=True,
    version=WITH_VERSION,
    classifiers=[
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Developers",
        "Intended Audience :: Information Technology",
        "Intended Audience :: Science/Research",
        "License :: OSI Approved :: MIT License",
        "Topic :: Software Development :: Libraries :: Python Modules",
        "Operating System :: POSIX :: Linux",
        "Operating System :: MacOS :: MacOS X",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
    ],
)
