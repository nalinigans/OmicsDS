/**
 * @file   omicsds_array_metadata.h
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2022 Omics Data Automation, Inc.
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
 * Header file for wrapper around array metadata protobuf
 */
#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

#include "omicsds_message_wrapper.h"

// Forward declaration of internal classes
class ArrayMetadata;
class Extent;
enum Dimension : int;

typedef std::pair<size_t, size_t> extents_t;

class OmicsDSArrayMetadata {
 public:
  OmicsDSArrayMetadata(std::string_view path);

  /**
   * Update the underlying array metadata from an existing metadata object.
   */
  void update_metadata(const std::shared_ptr<ArrayMetadata> metadata);

  /**
   * Returns whether or not the metadata has been initialized either by a call to init_metadata or
   * by parsing an existing file.
   */
  bool is_initialized();

  /**
   * Get the extents for the given dimension.
   */
  extents_t get_extent(Dimension dimension);

  /**
   * Expand the extent of the given dimension to include value.
   */
  extents_t expand_extent(Dimension dimension, size_t value);

 private:
  /**
   * Set up the mapping from from dimensions to extents.
   */
  void setup_extent_mapping();

  std::shared_ptr<OmicsDSMessage<ArrayMetadata>> m_metadata;
  std::string m_file_path;
  std::unordered_map<Dimension, Extent*> m_mapping;
  bool m_initialized = false;
};
