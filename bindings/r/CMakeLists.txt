#
# CMakeLists.txt
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

add_custom_target(omicsds_r
  COMMAND R CMD build ${CMAKE_CURRENT_SOURCE_DIR} --no-build-vignettes
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_dependencies(omicsds_r omicsds_static)

set(R_LIBS "~/R-libs" CACHE STRING "Directory to install R libraries in")

add_custom_target(omicsds_library_install
    COMMAND "${CMAKE_COMMAND}" --build "${PROJECT_BINARY_DIR}" --target install)
add_custom_target(omicsds_r_install
  COMMAND R -e \"install.packages('remotes', lib = '${R_LIBS}')\"
  COMMAND R -e \"library(remotes, lib.loc= '${R_LIBS}')\; remotes::install_local('${CMAKE_CURRENT_SOURCE_DIR}', lib = '${R_LIBS}', dependencies = TRUE, configure.args = '--with-omicsds=${CMAKE_INSTALL_PREFIX}')\"
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_dependencies(omicsds_r_install omicsds_r omicsds_library_install)
