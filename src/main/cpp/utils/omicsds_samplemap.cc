/**
 * @file   omicsds_samplemap.cc
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
 * Implementation for processing sample maps in OmicsDS
 */

#include "omicsds_samplemap.h"
#include "omicsds_file_utils.h"

SampleMap::SampleMap(const std::string& sample_map) {
  FileUtility file(sample_map);

  std::string line;
  while (file.generalized_getline(line)) {
    auto toks = split(line, "\t");
    if (toks.size() < 2) continue;
    try {
      std::string name = toks[0];
      size_t idx = std::stoul(toks[1]);
      if (!map.count(name)) {
        map[name] = idx;
      }
    } catch (...) {
      continue;
    }
  }
}

size_t& SampleMap::operator[](const std::string& name) { return map[name]; }

std::shared_ptr<std::unordered_map<size_t, std::string>> SampleMap::invert_sample_map(
    bool destructive) {
  std::unordered_map<size_t, std::string> inverted_sample_map =
      std::unordered_map<size_t, std::string>();
  auto sample_map_iter = map.begin();
  while (sample_map_iter != map.end()) {
    inverted_sample_map.insert({sample_map_iter->second, sample_map_iter->first});
    if (destructive) {
      sample_map_iter = map.erase(sample_map_iter);
    } else {
      sample_map_iter++;
    }
  }
  return std::make_shared<std::unordered_map<size_t, std::string>>(inverted_sample_map);
}
