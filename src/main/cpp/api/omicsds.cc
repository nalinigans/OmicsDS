/**
 * src/main/cpp/api/omicsds.cc
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
 * OmicsDS API Implementation
 */

#include "omicsds.h"
#include "omicsds_encoder.h"
#include "omicsds_export.h"
#include "omicsds_logger.h"

#include <map>
#include <mutex>

std::string OmicsDS::version() { return "0.0.1"; }

std::map<OmicsDSHandle, std::shared_ptr<OmicsExporter>> omicsds_instances{};
std::mutex omicsds_mutex;  // protects omicsds_instances
OmicsDSHandle current_handle = 0ul;

OmicsDSHandle OmicsDS::connect(const std::string& workspace, const std::string& array) {
  const std::lock_guard<std::mutex> lock(omicsds_mutex);
  std::shared_ptr<OmicsExporter> instance = std::make_shared<OmicsExporter>(workspace, array);
  omicsds_instances.emplace(std::piecewise_construct, std::make_tuple(current_handle),
                            std::make_tuple(instance));
  return current_handle++;
}

void OmicsDS::disconnect(OmicsDSHandle handle) {
  const std::lock_guard<std::mutex> lock(omicsds_mutex);
  omicsds_instances.erase(handle);
}

class FeatureProcessor {
 public:
  FeatureProcessor(std::vector<std::string>& features, std::array<int64_t, 2>& sample_range,
                   feature_process_fn_t proc)
      : m_features(features),
        m_process_all_features(!features.size()),
        m_sample_range(sample_range),
        m_proc(proc) {
    logger.info("Sample range lo={} hi={}", sample_range[0], sample_range[1]);
  }

  inline bool in_range(int64_t row) {
    return (m_sample_range[0] < 0 || m_sample_range[1] < 0) ||
           (row >= m_sample_range[0] && row <= m_sample_range[1]);
  }

  inline size_t remaining(const std::string& remove_feature) {
    auto found = std::find(m_features.begin(), m_features.end(), remove_feature);
    if (found != m_features.end()) {
      m_features.erase(found);
    }
    return m_features.size();
  }

  void process(const std::array<uint64_t, 3>& coords, const std::vector<OmicsFieldData>& data) {
    logger.debug("From cpp row_id={} feature_id={}", coords[0],
                 decode_gtf_id({coords[1], coords[2]}));
    auto& row_id = coords[0];
    if (in_range(row_id)) {
      auto gtf_id = decode_gtf_id({coords[1], coords[2]});
      // Heuristic to return early
      if (!m_process_all_features && gtf_id != last_feature_processed) {
        if (!remaining(last_feature_processed)) {
          // nothing left to process
          return;
        }
        last_feature_processed = gtf_id;
      }
      if (m_process_all_features ||
          std::find(m_features.begin(), m_features.end(), gtf_id) != m_features.end()) {
        float score = data[0].get<float>();
        if (m_proc) {
          m_proc(gtf_id, row_id, score);
        } else {
          logger.info("Feature id={}, Sample id={}, Score={}", gtf_id, row_id, score);
        }
      }
    }
  }

 private:
  std::vector<std::string>& m_features;
  bool m_process_all_features;
  std::array<int64_t, 2>& m_sample_range;
  feature_process_fn_t m_proc;

  std::string last_feature_processed;
};

void OmicsDS::query_features(OmicsDSHandle handle, std::vector<std::string>& features,
                             std::array<int64_t, 2>& sample_range, feature_process_fn_t proc) {
  std::shared_ptr<OmicsExporter> instance;
  {
    const std::lock_guard<std::mutex> lock(omicsds_mutex);
    instance = omicsds_instances.at(handle);
  }
  logger.debug("New Query");
  FeatureProcessor feature_processor(features, sample_range, proc);
  process_function bound = std::bind(&FeatureProcessor::process, std::ref(feature_processor),
                                     std::placeholders::_1, std::placeholders::_2);
  std::array<int64_t, 2> encoded_feature_range = {0, std::numeric_limits<int64_t>::max()};
  instance->query(sample_range, encoded_feature_range, bound);
}
