#
# CMakeLists.txt
#
# The MIT License
#
# Copyright (c) 2022 Omics Data Automation, Inc.
# Copyright (C) 2023 dātma, inc™
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

set(CPP_INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/src/main/cpp/loader
  ${CMAKE_SOURCE_DIR}/src/main/cpp/api
  ${CMAKE_SOURCE_DIR}/src/main/cpp/utils
  ${CMAKE_SOURCE_DIR}/src/test/cpp
  ${PROTOBUF_GENERATED_CXX_HDRS_INCLUDE_DIRS}
  )

set(LINK_DEPENDENCIES
  omicsds_static
  ${CATCH2_TARGET}
  ${OMICSDS_DEPENDENCIES}
  ${CMAKE_DL_LIBS})

# ctests for cli
set(OMICDS_TOOLS_DIR "${CMAKE_SOURCE_DIR}/tools/main")
set(CPP_CLI_TEST_SOURCES
  test_driver.cc
  test_import_config.cc
  test_omicsds_cli.cc)
add_executable(ctests_cli ${CPP_CLI_TEST_SOURCES} ${OMICDS_TOOLS_DIR}/cli_utils.cc)
target_include_directories(ctests_cli PRIVATE ${CPP_INCLUDE_DIRS} ${OMICDS_TOOLS_DIR})
target_compile_definitions(ctests_cli PRIVATE -DOMICSDS_TEST_INPUTS="${CMAKE_CURRENT_SOURCE_DIR}/../inputs/")
target_link_libraries(ctests_cli ${LINK_DEPENDENCIES})
add_test(ctests_cli ctests_cli -d yes)
set_target_properties(ctests_cli PROPERTIES EXCLUDE_FROM_ALL TRUE)

add_test(ctests_cli_build
  "${CMAKE_COMMAND}"
  --build "${CMAKE_BINARY_DIR}"
  --target ctests_cli
)

set_tests_properties(ctests_cli_build PROPERTIES FIXTURES_SETUP test_build_cli_fixture)
set_tests_properties(ctests_cli PROPERTIES FIXTURES_REQUIRED test_build_cli_fixture)
