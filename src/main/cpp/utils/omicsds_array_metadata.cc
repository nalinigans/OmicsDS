/**
 * @file   omicsds_array_metadata.cc
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2022 Omics Data Automation, Inc.
 * @copyright Copyright (c) 2023 dātma, inc™
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * @section DESCRIPTION
 *
 * Source file for wrapper around array metadata protobuf
 */

#include <functional>

#include "omicsds_array_metadata.h"
#include "omicsds_array_metadata.pb.h"
#include "omicsds_file_utils.h"
#include "omicsds_logger.h"

#include "tiledb_utils.h"

OmicsDSArrayMetadata::OmicsDSArrayMetadata(std::string_view path)
    : m_file_path(path), m_metadata(std::make_shared<OmicsDSMessage<ArrayMetadata>>(path)) {
  if (m_metadata->loaded_from_file()) setup_extent_mapping();
}

void OmicsDSArrayMetadata::setup_extent_mapping() {
  m_initialized = true;
  for (auto i = 0; i < m_metadata->message()->extents_size(); i++) {
    DimensionExtent* dimension_extent = m_metadata->message()->mutable_extents(i);
    m_mapping.emplace(dimension_extent->dimension(), dimension_extent->mutable_extent());
  }
}

extents_t OmicsDSArrayMetadata::get_extent(Dimension dimension) {
  Extent* extent = m_mapping.at(dimension);
  return std::make_pair(extent->start(), extent->end());
}

extents_t OmicsDSArrayMetadata::expand_extent(Dimension dimension, size_t value) {
  Extent* extent = m_mapping.at(dimension);
  if (value < extent->start()) extent->set_start(value);
  if (value > extent->end()) extent->set_end(value);
  return get_extent(dimension);
}

void OmicsDSArrayMetadata::update_metadata(const std::shared_ptr<ArrayMetadata> metadata) {
  m_metadata->message()->CopyFrom(*metadata);
  setup_extent_mapping();
}

bool OmicsDSArrayMetadata::is_initialized() { return m_initialized; }
