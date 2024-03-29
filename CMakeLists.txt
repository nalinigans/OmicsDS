#
# CMakeLists.txt
#
# The MIT License
#
# Copyright (c) 2022 Omics Data Automation, Inc.
# Copyright (c) 2023 dātma, inc™
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

# External dependencies. Can be overridden to point to custom locations
set(HTSLIB_SOURCE_DIR "${CMAKE_SOURCE_DIR}/dependencies/htslib" CACHE PATH "Path to htslib source directory")
set(TILEDB_SOURCE_DIR "${CMAKE_SOURCE_DIR}/dependencies/TileDB" CACHE PATH "Path to TileDB source directory")
set(SPDLOG_INCLUDE_DIR  "${CMAKE_SOURCE_DIR}/dependencies/spdlog/include" CACHE PATH "Path to SpdLog include directory")

# Protobuf library
set(OMICSDS_PROTOBUF_VERSION "3.21.7" CACHE STRING "Version of Google Protocol Buffer library")
set(PROTOBUF_ROOT_DIR "$ENV{HOME}/protobuf-install" CACHE PATH "Path to installed protobuf")
set(PROTOBUF_URL "https://github.com/protocolbuffers/protobuf/archive/v${OMICSDS_PROTOBUF_VERSION}.zip" CACHE STRING "URL to protobuf github release")

# GIT_COMMIT_HASH derived from git tag to be used as part of OMICSDS_VERSION
execute_process(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

# Build parameters/options
set(OMICSDS_RELEASE_VERSION "0.0.1-SNAPSHOT" CACHE STRING "OmicsDS release version")
set(OMICSDS_VERSION "${OMICSDS_RELEASE_VERSION}-${GIT_COMMIT_HASH}" CACHE STRING "OmicsDS full version string")

if(APPLE)
  set(DISABLE_OPENMP True CACHE BOOL "Disable OpenMP")
else()
  set(DISABLE_OPENMP False CACHE BOOL "Disable OpenMP")
endif()

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(HTSLIB_INSTALL_DIR "" CACHE PATH "Path to htslib install directory")

# Honor visibility properties for all target types.
# https://www.cmake.org/cmake/help/latest/policy/CMP0063.html
cmake_policy(SET CMP0063 NEW)

# The DOWNLOAD_EXTRACT_TIMESTAMP option to the ExternalProject_Add() command is used to explicitly specify
# how timestamps should be handled. The default behavior is to set the timestamps of extracted contents to
# the time of extraction.
# https://cmake.org/cmake/help/latest/policy/CMP0135.html#policy:CMP0135
if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

# See https://cmake.org/Wiki/CMake_RPATH_handling#Common_questions
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH True)
if(APPLE)
  set(CMAKE_MACOSX_RPATH True CACHE BOOL "Set rpath on OSX")
  set(CMAKE_FIND_FRAMEWORK LAST CACHE STRING "Try to find frameworks on Mac OS X last")
endif()

# Set C++17 as required standard for all C++ targets.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optimization flag
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND BUILD_FOR_ARCH)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=${BUILD_FOR_ARCH}")
endif()

# Set stack smashing protection - emits extra code for buffer overlows including local frames.
# Should check how much of a performance hit this causes.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong")

# Set -fvisibility=hidden
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

# Compiler Warning Options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-reorder -Wno-unknown-pragmas -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-result")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -z noexecstack -z relro -z now")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -z noexecstack -z relro -z now")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-reorder -Wno-unused-variable -Wno-pessimizing-move -Wno-deprecated-declarations")
endif()

# Honor visibility properties for all target types.
# https://www.cmake.org/cmake/help/latest/policy/CMP0063.html
#cmake_policy(SET CMP0063 NEW)

# Collect profiling stats to use with gperf/pprof
# To install gperftools - sudo yum -y install gperftools - Centos 7
#                         sudo apt-get install -y google-perftools - Ubuntu
#                         brew install gperftools - Mac
# See https://gperftools.github.io/gperftools/cpuprofile.html
option(USE_GPERFTOOLS "Collect profiling information using gperftools/profiler for analysis with pprof")
# See https://gperftools.github.io/gperftools/heapprofile.html
option(USE_GPERFTOOLS_HEAP "Collect heap profiling information using gperftools/tcmalloc for analysis with pprof")
if(USE_GPERFTOOLS OR USE_GPERFTOOLS_HEAP)
    find_package(GPerftools)
    if(GPERFTOOLS_FOUND OR GPERFTOOLS_HEAP_FOUND)
        message(STATUS "Enabling profiling using GPerftools")
        include_directories(${GPERFTOOLS_INCLUDE_DIR})
        if(USE_GPERFTOOLS)
            add_definitions(-DUSE_GPERFTOOLS)
        endif()
        if(USE_GPERFTOOLS_HEAP)
            add_definitions(-DUSE_GPERFTOOLS_HEAP)
        endif()
    else()
        message(WARNING "GPerftools headers/library not found")
    endif()
endif()

find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(libuuid REQUIRED)
if (NOT DISABLE_OPENMP)
  find_package(OpenMP REQUIRED)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# SpdLog
find_package(Spdlog REQUIRED)
include_directories(${SPDLOG_INCLUDE_DIR})

# htslib
find_package(htslib REQUIRED)
include_directories(BEFORE ${HTSLIB_INCLUDE_DIR})

# TileDB
find_package(TileDB REQUIRED)
include_directories(${TILEDB_INCLUDE_DIR})

#Protobuf library
find_package(ProtobufWrapper REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

# Catch2
include(SetCatch2Version)

set(OMICSDS_DEPENDENCIES
  ${HTSLIB_LIBRARY}
  tiledb_static
  ${OPENSSL_LIBRARIES}
  ${ZLIB_LIBRARIES}
  ${LIBUUID_LIBRARY}
  ${PROTOBUF_LIBRARIES}
  -ldl
)

set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -g3 -gdwarf-3")
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3 -fvisibility=hidden")
set(CMAKE_CXX_FLAGS_COVERAGE "-DDEBUG -g3 -gdwarf-3 --coverage")

include_directories(src/loader)

# Build and Test OmicsDS
enable_testing()
add_subdirectory(src)

# Build tools
add_subdirectory(tools)

# Bindings
add_subdirectory(bindings)

# Docs
if (DEFINED BUILD_DOCS)
  add_subdirectory(docs)
endif()

# Unininstall OmicsDS
add_custom_target(uninstall-omicsds
  COMMAND echo "-- Uninstalling OmicsDS from ${CMAKE_INSTALL_PREFIX}..."
  COMMAND < install_manifest.txt xargs -I % sh -c 'echo -- Uninstalling % && rm -f %' %
  COMMAND echo "-- OmicsDS is uninstalled!"
)
