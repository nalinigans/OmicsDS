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

set(PROTOBUF_PROTO_FILES
  omicsds_array_metadata.proto
  )

set(PROTO_SRC_DIR ${CMAKE_CURRENT_BINARY_DIR})

foreach(PROTO_FILE ${PROTOBUF_PROTO_FILES})
  get_filename_component(CURR_FILENAME ${PROTO_FILE} NAME_WE)
  list(APPEND PROTO_GENERATED_CXX_SRCS ${PROTO_SRC_DIR}/${CURR_FILENAME}.pb.cc)
  list(APPEND PROTO_GENERATED_CXX_HDRS ${PROTO_SRC_DIR}/${CURR_FILENAME}.pb.h)
endforeach()

set(PROTOBUF_GENERATED_CXX_HDRS_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR})
add_custom_command(OUTPUT ${PROTO_GENERATED_CXX_SRCS} ${PROTO_GENERATED_CXX_HDRS}
  COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --cpp_out=${CMAKE_CURRENT_BINARY_DIR} ${PROTOBUF_PROTO_FILES}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  BYPRODUCTS ${PROTO_GENERATED_CXX_SRCS} ${PROTO_GENERATED_CXX_HDRS}
  COMMENT "Generating source/header files from .proto files"
  DEPENDS protobuf_ep ${PROTOBUF_PROTO_FILES})
add_custom_target(PROTOBUF_GENERATED_CXX_TARGET DEPENDS
  ${PROTO_GENERATED_CXX_SRCS} ${PROTO_GENERATED_CXX_HDRS} ${PROTO_GENERATED_PYTHON_SRCS})

set(PROTOBUF_GENERATED_CXX_SRCS ${PROTO_GENERATED_CXX_SRCS} CACHE INTERNAL "Protocol buffers generated C++ sources")
set(PROTOBUF_GENERATED_CXX_HDRS ${PROTO_GENERATED_CXX_HDRS} CACHE INTERNAL "Protocol buffers generated C++ headers")
set(PROTOBUF_GENERATED_CXX_HDRS_INCLUDE_DIRS ${PROTOBUF_GENERATED_CXX_HDRS_INCLUDE_DIRS} CACHE INTERNAL "Directory containing Protocol buffers generated C++ headers")
