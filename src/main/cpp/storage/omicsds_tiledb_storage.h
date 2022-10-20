/**
 * src/main/cpp/storage/omicsds_tiledb_storage.h
 *
 * The MIT License (MIT)
 * Copyright (c) 2022 Omics Data Automation, Inc.
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
 * TileDB header for omicds storage.
 */

#pragma once

#include "omicsds_file_utils.h"
#include "omicsds_storage.h"

#include "tiledb.h"

class TileDBArrayStorage : public OmicsDSArrayStorage, public OmicsDSTileDBUtils {
 public:
  TileDBArrayStorage(std::string_view workspace = "workspace", std::string_view array = "array")
      : OmicsDSArrayStorage(workspace, array),
        m_array_path(FileUtility::append(workspace, array)) {}

  ~TileDBArrayStorage() override { finalize(); }

  void configure_workspace() override;

  void initialize(bool read_only, std::shared_ptr<OmicsSchema> schema, bool overwrite_workspace,
                  bool overwrite_array) override;
  void finalize() override;

  void reopen_array() override;

  int store(std::vector<void*>& buffers, std::vector<size_t>& buffer_sizes) override;
  int retrieve(std::vector<void*>& buffers, std::vector<size_t>& buffer_sizes) override;
  int retrieve_by_cell(std::vector<void*>& buffers, std::vector<size_t>& buffer_sizes,
                       int64_t* subarray, process_cell_t processor) override;

  int consolidate() override;

  int to_field_type(OmicsFieldInfo::OmicsFieldType omics_type) override;

 private:
  void open_array(bool write_mode);

  TileDB_CTX* m_tiledb_ctx = nullptr;
  TileDB_Array* m_tiledb_array = nullptr;
  std::string m_array_path;
};
