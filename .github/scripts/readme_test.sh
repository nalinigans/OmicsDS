# readme_test.sh
#
# The MIT License
#
# Copyright (c) 2022 Omics Data Automation, Inc.
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
# Runs through all the commands in the README to check that everything is working as expected

#!/bin/bash
set -e

VIRTUAL_ENV=${VIRTUAL_ENV:-~/.env}
OMICSDS_REPO=${OMICSDS_REPO:-~/OmicsDS}
BUILD_DIR=${BUILD_DIR:-${OMICSDS_REPO}/build}
INSTALL_DIR=${INSTALL_DIR:-${OMICSDS_REPO}/install}
R_LIBS=${R_LIBS:-~/R-libs}

. ${VIRTUAL_ENV}/bin/activate

if [ -d "${BUILD_DIR}" ]; then
  rm -r ${BUILD_DIR}
fi

if [ ! -d "${R_LIBS}" ]; then
  mkdir -p ${R_LIBS}
fi

mkdir ${BUILD_DIR}
cd ${BUILD_DIR}

# Initial configuration # TODO fix prefix path
cmake ${OMICSDS_REPO} -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} -DR_LIBS=${R_LIBS} -DBUILD_DOCS=TRUE -DCMAKE_PREFIX_PATH=~/catch2-install

# Library build
cmake --build ${BUILD_DIR} --target install --parallel $(nproc)

# Tests build
cmake --build ${BUILD_DIR} --target test --parallel $(nproc)

# Docs build
cmake --build ${BUILD_DIR} --target docs --parallel $(nproc)

# Bindings build
make omicsds_r_install
make omicsds_python
