/**
 * src/main/cpp/api/omicsds.h
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
 * Interface to OmicsDS
 */

#pragma once

#include <array>
#include <functional>
#include <string>
#include <vector>

// Override project visibility set to hidden for api
#if (defined __GNUC__ && __GNUC__ >= 4) || defined __INTEL_COMPILER
#define OMICSDS_EXPORT __attribute__((visibility("default")))
#else
#define OMICSDS_EXPORT
#endif

typedef size_t OmicsDSHandle;

typedef std::function<void(const std::string& feature_id, uint64_t sample_id, float score)>
    feature_process_fn_t;

class OMICSDS_EXPORT OmicsDS {
 public:
  // Utilities
  static std::string version();
  static OmicsDSHandle connect(const std::string& workspace, const std::string& array);
  static void disconnect(OmicsDSHandle handle);
  static void query_features(OmicsDSHandle handle, std::vector<std::string>& features,
                             std::array<int64_t, 2>& sample_range,
                             feature_process_fn_t proc = NULL);
};
