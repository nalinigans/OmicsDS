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

project(OmicsDS)
cmake_minimum_required(VERSION 3.6)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/Modules")

if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  message(FATAL_ERROR CMAKE_CXX_COMPILER_ID " Compiler not yet supported. Exiting.")
endif()

#External dependencies. Can be overridden to point to custom locations
set(HTSLIB_SOURCE_DIR "${CMAKE_SOURCE_DIR}/dependencies/htslib" CACHE PATH "Path to htslib source directory")
set(TILEDB_SOURCE_DIR "${CMAKE_SOURCE_DIR}/dependencies/TileDB" CACHE PATH "Path to TileDB source directory")
set(SPDLOG_INCLUDE_DIR  "${CMAKE_SOURCE_DIR}/dependencies/spdlog/include" CACHE PATH "Path to SpdLog include directory")
#The fmt headers are in dependencies/spdlog/include/spdlog/fmt/bundled - source files will include "fmt/bundled/format.h"
set(FMT_ROOT_DIR "${SPDLOG_INCLUDE_DIR}" CACHE PATH "Path to fmt library")
set(USING_FMT_BUNDLED_WITH_SPDLOG True CACHE BOOL "Disable if using fmt library that's not bundled with spdlog")

#GIT_COMMIT_HASH derived from git tag to be used as part of OMICSDS_VERSION
execute_process(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

#Build parameters/options
set(OMICSDS_RELEASE_VERSION "0.0.1-SNAPSHOT" CACHE STRING "OmicsDS release version")
set(OMICSDS_VERSION "${OMICSDS_RELEASE_VERSION}-${GIT_COMMIT_HASH}" CACHE STRING "OmicsDS full version string")
