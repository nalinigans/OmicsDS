/**
 * src/main/cpp/storage/omicsds_storage.h
 *
 * The MIT License (MIT)
 * Copyright (c) 2022 Omics Data Automation, Inc.
 * @copyright Copyright (c) 2023 dātma, inc™
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Common header for all supported storage libraries, e.g. TileDB.
 */

#pragma once

#include "omicsds_schema.h"

#include <functional>
#include <string>

// TODO Move the FileSystem operations here
class OmicsDSFilesystem {};

// TODO: replace process_function typedef with this eventually
typedef std::function<void(const std::array<uint64_t, 3>& coords,
                           const std::vector<OmicsFieldData>& data)>
    process_cell_t;

class OmicsDSArrayStorage {
 public:
  OmicsDSArrayStorage(std::string_view workspace = "workspace", std::string_view array = "array")
      : m_workspace(workspace), m_array(array) {}
  virtual ~OmicsDSArrayStorage() {}

  // Delete copy constructors
  OmicsDSArrayStorage(const OmicsDSArrayStorage& other) = delete;
  OmicsDSArrayStorage operator=(const OmicsDSArrayStorage& other) = delete;

  // Configure workspace
  virtual void configure_workspace() {}

  /** create workspace/array */
  void initialize() { initialize(false, nullptr, false, false); }
  void initialize(bool write_mode) { initialize(write_mode, nullptr, false, false); }
  // TODO: allow for optional protobuf configuration to be passed in here as well
  virtual void initialize(bool write_mode, std::shared_ptr<OmicsSchema> schema,
                          bool overwrite_workspace, bool overwrite_array) {}
  virtual void finalize() {}

  virtual void reopen_array() {}

  virtual int store(std::vector<void*>& buffers, std::vector<size_t>& buffer_sizes) { return 0; }
  virtual int retrieve(std::vector<void*>& buffers, std::vector<size_t>& buffer_sizes) { return 0; }
  virtual int retrieve_by_cell(std::vector<void*>& buffers, std::vector<size_t>& buffer_sizes,
                               int64_t* subarray, process_cell_t processor) {
    return 0;
  }
  virtual int consolidate() { return 0; }

  virtual int to_field_type(OmicsFieldInfo::OmicsFieldType omics_type) { return omics_type; }

 protected:
  std::string m_workspace;
  std::string m_array;
};
