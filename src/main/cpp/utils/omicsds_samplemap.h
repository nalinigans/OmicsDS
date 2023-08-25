/**
 * @file   omicsds_samplemap.h
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
 * Header file for processing sample maps in OmicsDS
 */

#pragma once

#include <map>
#include <memory>
#include <string>
#include <unordered_map>

// maps from sample name to (logical) row in OmicsDS
struct SampleMap {
  std::map<std::string, size_t> map;
  // file specifying SampleMap should be tab seperated with lines
  // consisting of a single sample name and row index
  SampleMap(const std::string& sample_map);
  size_t& operator[](const std::string& name);
  size_t count(const std::string& name) const { return map.count(name); }
  size_t size() const { return map.size(); }
  /**
   * Generates the inverse of this sample map, optionally erasing the original map in the process.
   */
  std::shared_ptr<std::unordered_map<size_t, std::string>> invert_sample_map(
      bool destructive = false);
};
