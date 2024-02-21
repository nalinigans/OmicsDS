/**
 * omicsds_processor.cc
 *
 * Copyright (c) 2023-2024 dātma, inc™
 *
 * Proprietary and Confidential:
 * Unauthorized copying of this file by any medium is strictly prohibited.
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
