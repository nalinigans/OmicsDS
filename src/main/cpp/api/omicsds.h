/**
 * src/main/cpp/api/omicsds.h
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
 * Interface to OmicsDS
 */

#pragma once

#include <array>
#include <functional>
#include <string>
#include <utility>
#include <vector>

// Override project visibility set to hidden for api
#if (defined __GNUC__ && __GNUC__ >= 4) || defined __INTEL_COMPILER
#define OMICSDS_EXPORT __attribute__((visibility("default")))
#else
#define OMICSDS_EXPORT
#endif

/**
 * Handle representing a workspace and array pair that the library has allocated resources for
 * operating on.
 */
typedef size_t OmicsDSHandle;

/**
 * A function definition for processing entries in a feature matrix.
 */
typedef std::function<void(const std::string& feature_id, uint64_t sample_id, float score)>
    feature_process_fn_t;

class OMICSDS_EXPORT OmicsDS {
 public:
  // Utilities
  /**
   * Returns a string representation of the current OmicsDS library version.
   *
   * @return the current library version
   */
  static std::string version();
  /**
   * Connects to a given workspace and array, returning a handle for subsequent operations.
   *
   * @param workspace the path to the workspace to connect to
   * @param array     the array within the specificed workspace to connect to
   * @return          a handle for use in subsequent requests
   */
  static OmicsDSHandle connect(const std::string& workspace, const std::string& array);
  /**
   * Disconnects the given handle, freeing any resources.
   *
   * @param handle the handle that should be disconnected
   */
  static void disconnect(OmicsDSHandle handle);
  /**
   * Query a given handle, processing the results.
   *
   * @param handle       a handle previously returned by OmicsDS::connect
   * @param features     the set of features to query on
   * @param sample_range the range of samples to query on inclusive of both endpoints
   * @param proc         a function that will process each feature sample pair as it is queried. By
   * default, results will be printed to stdout
   */
  static void query_features(OmicsDSHandle handle, std::vector<std::string>& features,
                             std::array<int64_t, 2>& sample_range,
                             feature_process_fn_t proc = NULL);

  /**
   * Query a given handle, processing the results.
   *
   * @param handle       a handle previously returned by OmicsDS::connect
   * @param features     the set of features to query on
   * @param sample_range the range of samples to query on inclusive of both endpoints
   * @param proc         a function that will process each feature sample pair as it is queried. By
   * default, results will be printed to stdout
   */
  static void query_features(OmicsDSHandle handle, std::vector<std::string>& features,
                             std::pair<int64_t, int64_t>& sample_range,
                             feature_process_fn_t proc = NULL);
};
