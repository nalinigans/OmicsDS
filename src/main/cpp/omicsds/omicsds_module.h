/**
 * src/main/cpp/omicsds/omicsds_module.cc
 *
 * The MIT License (MIT)
 * Copyright (c) 2022 Omics Data Automation, Inc.
 * @copyright Copyright (C) 2023 dātma, inc™
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
 * Header for OmicsDS loader/export/consolidate Modules
 */

#pragma once

#include "omicsds_array_metadata.h"
#include "omicsds_file_utils.h"
#include "omicsds_schema.h"
#include "omicsds_tiledb_storage.h"

#include <string>

// Base Module class for Loader and Exporter functionality
class OmicsDSModule {
 public:
  // Constructor for retrieving/exporting operations
  OmicsDSModule(std::string_view workspace, std::string_view array)
      : m_array_storage(std::make_shared<TileDBArrayStorage>(workspace, array)),
        m_schema_default_path(FileUtility::append(workspace, array, "omics_schema")) {
    m_array_storage->initialize();
  }

  // Constructor for initial loading/importing operations
  // TODO: Support incremental loading/importing
  OmicsDSModule(std::string_view workspace, std::string_view array, std::string_view mapping_file,
                bool position_major)
      : m_array_storage(std::make_shared<TileDBArrayStorage>(workspace, array)),
        m_schema_default_path(FileUtility::append(workspace, array, "omics_schema")),
        m_schema(std::make_shared<OmicsSchema>(std::string(mapping_file), position_major)),
        m_array_metadata(std::make_shared<OmicsDSArrayMetadata>(
            FileUtility::append(workspace, array, "metadata"))) {}

  void reopen_array() { m_array_storage->reopen_array(); }

  void serialize_schema(std::string path) { m_schema->serialize(path); }

  void serialize_schema() { serialize_schema(m_schema_default_path); }

  void deserialize_schema(std::string path) {
    m_schema.reset();
    m_schema = std::make_shared<OmicsSchema>();
    m_schema->create_from_file(path);
  }

  void deserialize_schema() { deserialize_schema(m_schema_default_path); };

 protected:
  std::shared_ptr<OmicsSchema> m_schema;
  std::string m_schema_default_path;
  std::shared_ptr<OmicsDSArrayStorage> m_array_storage;
  std::shared_ptr<OmicsDSArrayMetadata> m_array_metadata;
};
