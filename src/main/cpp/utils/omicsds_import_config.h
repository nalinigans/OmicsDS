/**
 * @file   omicsds_import_config.h
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
 * Header file for wrapper around protobuf import configuration
 */

#pragma once

#include <optional>
#include <string>

// Import Types
enum OmicsDSImportType {
  FEATURE_IMPORT,
  READ_IMPORT,
  INTERVAL_IMPORT,
};

// Configuration parameters
struct OmicsDSImportConfig {
  std::optional<OmicsDSImportType> import_type;
  std::optional<std::string> file_list;
  std::optional<std::string> sample_map;
  std::optional<std::string> mapping_file;
  bool sample_major = false;

  /**
   * Update this OmicsDSImportConfig, using set fields in update_config.
   *
   * Any fields which are set (the std::optional contains a value rather than a std::nullopt_t) in
   * update_config will be used to overwrite the value of the corresponding field in the
   * OmicsDSImportConfig instance this function is called on.
   *
   * \param update_config OmicsDSImportConfig instance to read updated values from.
   */
  void merge(const OmicsDSImportConfig& update_config);
};
