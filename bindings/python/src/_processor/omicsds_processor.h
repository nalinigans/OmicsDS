/**
 * omicsds_processor.h
 *
 * Copyright (c) 2023-2024 dātma, inc™
 *
 * Proprietary and Confidential:
 * Unauthorized copying of this file by any medium is strictly prohibited.
 */

#include <stdint.h>
#include <string>
#include <unordered_set>
#include <vector>

class OmicsDSProcessor {
 public:
  OmicsDSProcessor(std::vector<std::string>* features, std::vector<std::string>* samples,
                   std::vector<float>* scores, std::string sample_map = "");
  // Functor - makes class OmicsDSProcessor behave like a function
  void operator()(const std::string& feature_id, uint64_t sample_id, float score);

 private:
  std::vector<std::string>* m_features;
  std::unordered_set<std::string> m_seen_features;
  std::vector<std::string>* m_samples;
  std::unordered_set<uint64_t> m_seen_samples;
  std::vector<float>* m_scores;
  std::shared_ptr<std::unordered_map<size_t, std::string>> m_sample_map = nullptr;
};
