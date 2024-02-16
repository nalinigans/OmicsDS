/**
 * src/main/cpp/loader/omicsds_export.h
 *
 * The MIT License (MIT)
 * Copyright (c) 2022 Omics Data Automation, Inc.
 * @copyright Copyright (c) 2024 dātma, inc™
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
 * Specification for OmicsDS Export funtionality
 */

#pragma once

#include "omicsds_module.h"

#include <functional>
#include <limits>

typedef std::function<void(const std::array<uint64_t, 3>& coords,
                           const std::vector<OmicsFieldData>& data)>
    process_function;

// used to query from OmicsDS
class OmicsExporter : public OmicsDSModule {
 public:
  OmicsExporter(const std::string& workspace, const std::string& array)
      : OmicsDSModule(workspace, array) {
    deserialize_schema();
  }

  virtual ~OmicsExporter() {}

  // used to query given range
  // will use proc as callback if specified, otherwise will default to process
  void query(std::array<int64_t, 2> sample_range = {0, std::numeric_limits<int64_t>::max()},
             std::array<int64_t, 2> position_range = {0, std::numeric_limits<int64_t>::max()},
             process_function proc = 0);

 protected:
  // coords are in standard order SAMPLE, POSITION, COLLISION INDEX
  virtual void process(const std::array<uint64_t, 3>& coords,
                       const std::vector<OmicsFieldData>& data);
  std::vector<std::vector<uint8_t>> m_buffers_vector;
  std::pair<std::vector<void*>, std::vector<size_t>> prepare_buffers();
  size_t m_buffer_size = 10240;
  void check(const std::string& name,
             const OmicsFieldInfo& inf);  // check that an attribute exists in schema (useful for
                                          // specific data e.g. ensure that the data is actually
                                          // from readcounts/sam files before exporting
};

// for exporting data as SAM files
// will create one per row with name sam_output[row idx].sam
// rows with no data in query range will not appear in output
class SamExporter : public OmicsExporter {  // for exporting data as SAM files
 public:
  SamExporter(const std::string& workspace, const std::string& array);
  void export_sams(std::array<int64_t, 2> sample_range = {0, std::numeric_limits<int64_t>::max()},
                   std::array<int64_t, 2> position_range = {0, std::numeric_limits<int64_t>::max()},
                   const std::string& ouput_prefix = "sam_output");

 protected:
  // callback to write to sam files
  void sam_interface(std::map<int64_t, std::shared_ptr<std::ofstream>>& files,
                     const std::string& output_prefix, const std::array<uint64_t, 3>& coords,
                     const std::vector<OmicsFieldData>& data);
};
