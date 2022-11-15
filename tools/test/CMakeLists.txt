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

add_subdirectory(cpp)
add_test(NAME cli_tools_test
         COMMAND test_cli.sh ${CMAKE_SOURCE_DIR}/src/test/inputs/OmicsDSTests ${CMAKE_CURRENT_BINARY_DIR}/../main
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_test(ctests_tool_build
  "${CMAKE_COMMAND}"
  --build "${CMAKE_BINARY_DIR}"
  --target omicsds
)

set_tests_properties(ctests_tool_build PROPERTIES FIXTURES_SETUP test_build_tool_fixture)
set_tests_properties(cli_tools_test PROPERTIES FIXTURES_REQUIRED test_build_tool_fixture)
