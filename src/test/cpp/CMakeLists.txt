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


include_directories(Catch INTERFACE Catch2/single_include)

SET(CPP_INCLUDE_DIRS
        ${CMAKE_SOURCE_DIR}/src/main/cpp/loader
        ${CMAKE_SOURCE_DIR}/src/main/cpp/utils)
SET(LINK_DEPENDENCIES
        omicsds_static
        ${OMICSDS_DEPENDENCIES}
        ${CMAKE_DL_LIBS})
set(CPP_TEST_SOURCES
        test_omicsds_loader.cc
        test_omics_field_data.cc
        test_file_utility.cc
        test_logger.cc
        test_encoder.cc)
add_executable(ctests_lib ${CPP_TEST_SOURCES})
target_include_directories(ctests_lib PRIVATE ${CPP_INCLUDE_DIRS})
target_compile_definitions(ctests_lib PRIVATE -DOMICSDS_TEST_INPUTS="${CMAKE_CURRENT_SOURCE_DIR}/../inputs/")
target_link_libraries(ctests_lib ${LINK_DEPENDENCIES})
add_test(ctests_lib ctests_lib -d yes)

set(OMICDS_TOOLS_DIR "${CMAKE_SOURCE_DIR}/tools")
set(CPP_CLI_TEST_SOURCES
        test_omicsds_cli.cc)
add_executable(ctests_cli ${CPP_CLI_TEST_SOURCES} ${OMICDS_TOOLS_DIR}/cli_utils.cc)
target_include_directories(ctests_cli PRIVATE ${CPP_INCLUDE_DIRS} ${OMICDS_TOOLS_DIR})
target_link_libraries(ctests_cli ${LINK_DEPENDENCIES})
add_test(ctests_cli ctests_cli -d yes)