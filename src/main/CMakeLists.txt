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

set(OMICSDS_CPP "${CMAKE_CURRENT_SOURCE_DIR}/cpp")

set(OMICSDS_API_INCLUDE
  ${OMICSDS_CPP}/api/omicsds.h
  )

set(OMICSDS_INCLUDE_DIRS
  ${OMICSDS_CPP}/omicsds
  ${OMICSDS_CPP}/utils
  ${OMICSDS_CPP}/storage
  ${OMICSDS_CPP}/api
  )

set(OMICSDS_SOURCES
  ${OMICSDS_CPP}/omicsds/omicsds_loader.cc
  ${OMICSDS_CPP}/omicsds/omicsds_export.cc
  ${OMICSDS_CPP}/omicsds/omicsds_configure.cc
  ${OMICSDS_CPP}/storage/omicsds_tiledb_storage.cc
  ${OMICSDS_CPP}/utils/omicsds_encoder.cc
  ${OMICSDS_CPP}/utils/omicsds_logger.cc
  ${OMICSDS_CPP}/utils/omicsds_file_utils.cc
  ${OMICSDS_CPP}/utils/omicsds_samplemap.cc
  ${OMICSDS_CPP}/utils/omicsds_array_metadata.cc
  ${OMICSDS_CPP}/utils/omicsds_message_wrapper.cc
  ${OMICSDS_CPP}/utils/omicsds_import_config.cc
  ${OMICSDS_CPP}/api/omicsds.cc
  ${PROTOBUF_GENERATED_CXX_SRCS}
  )

set_source_files_properties(${PROTOBUF_GENERATED_CXX_SRCS} ${PROTOBUF_GENERATED_CXX_HDRS} PROPERTIES GENERATED True)

# Use PIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
add_library(omicsds_objs OBJECT ${OMICSDS_SOURCES})
add_dependencies(omicsds_objs PROTOBUF_GENERATED_CXX_TARGET)

target_include_directories(omicsds_objs
  PUBLIC ${CMAKE_BINARY_DIR}/src
  PRIVATE ${OMICSDS_INCLUDE_DIRS} ${TILEDB_INCLUDE_DIR} ${PROTOBUF_GENERATED_CXX_HDRS_INCLUDE_DIRS}
  )

# Create static library
add_library(omicsds_static STATIC $<TARGET_OBJECTS:omicsds_objs>)
set_target_properties(omicsds_static PROPERTIES OUTPUT_NAME "omicsds" VERSION ${OMICSDS_VERSION})
add_dependencies(omicsds_static htslib)
target_link_libraries(omicsds_static
  tiledb_static
  ${OMICSDS_DEPENDENCIES}
  ${CMAKE_DL_LIBS} # add -ldl
  ) 

# Create shared library
add_library(omicsds_shared SHARED $<TARGET_OBJECTS:omicsds_objs>)
set_target_properties(omicsds_shared PROPERTIES OUTPUT_NAME "omicsds" VERSION ${OMICSDS_VERSION} SOVERSION 1)
add_dependencies(omicsds_shared htslib)
target_link_libraries(omicsds_shared 
  tiledb_static
  ${OMICSDS_DEPENDENCIES}
  pthread
  ${CMAKE_DL_LIBS} # add -ldl
  )

# Install libraries
install(
  TARGETS omicsds_static omicsds_shared
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )

#install include files
install(
  FILES ${OMICSDS_API_INCLUDE} ${EXPORTS_HEADER} DESTINATION include/omicsds
  )
