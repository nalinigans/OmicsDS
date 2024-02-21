/**
 * omicsds_processor.cc
 *
 * The MIT License (MIT)
 * Copyright (c) 2023 Omics Data Automation, Inc.
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
 */
#include "omicsds_processor.h"
#include "omicsds.h"
#include "stdio.h"

OmicsDSProcessor::OmicsDSProcessor(std::vector<std::string>* features,
                                   std::vector<std::string>* samples, std::vector<float>* scores,
                                   std::string sample_map)
    : m_features(features), m_samples(samples), m_scores(scores) {
  if (!sample_map.empty()) {
    m_sample_map = OmicsDS::resolve_sample_map(sample_map);
  }
}

void OmicsDSProcessor::operator()(const std::string& feature_id, uint64_t sample_id, float score) {
  // Insert feature_id into results
  if (m_seen_features.find(feature_id) == m_seen_features.end()) {
    m_features->emplace_back(feature_id);
    m_seen_features.emplace(feature_id);
  }

  // Insert sample_id into results
  if (m_seen_samples.find(sample_id) == m_seen_samples.end()) {
    if (m_sample_map && !m_sample_map->at(sample_id).empty()) {
      m_samples->emplace_back(m_sample_map->at(sample_id));
    } else {
      m_samples->emplace_back(std::to_string(sample_id));
    }

    m_seen_samples.emplace(sample_id);
  }

  // Insert score into results
  m_scores->push_back(score);
}
