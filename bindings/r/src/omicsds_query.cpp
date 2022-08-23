/**
 * @file omicsds_query.cpp
 *
 * @section LICENSE
 *
 * The MIT License (MIT)
 *
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
 * @section DESCRIPTION
 *
 * Rcpp code to interface with the omicsds library
 *
 **/

#include <RcppCommon.h>
#include "omicsds_types.h"

// Custom "wrap" and "as" templates are required to be between RcppCommon.h and Rcpp.h
namespace Rcpp
{
  // non-intrusive extension via template specialisation
  template <>
  std::array<int64_t, 2> as(SEXP range);
}


#include <Rcpp.h>
#include <exception>
#include <iostream>

template <>
std::array<int64_t, 2> Rcpp::as(SEXP range) {
  std::array<int64_t, 2> range_array = {0, std::numeric_limits<int64_t>::max()};
  if (Rcpp::Nullable<Rcpp::List>(range).isNotNull()) {
    Rcpp::List range_list(range);
    range_array[0] = range_list[0];
    if (range_list.size() == 1) {
      range_array[1] = range_list[0];
    } else {
      range_array[1] = range_list[1];
    }
  }
  return range_array;
}

//' Version of OmicsDS
// [[Rcpp::export]]
Rcpp::CharacterVector version() {
  return OmicsDS::version();
}

//' Connect to an OmicsDS workspace/array
//'
//' Get a handle to an OmicsDS instance given a workspace/array
//' Calling disconnect will release resources held
//' by the native OmicsDS.
//'
//' @param workspace path to an OmicsDS workspace
//' @param array name of the array in the OmicsDS workspace
//' @return handle representation of the OmicsDS instance for the given workspace and array
//' @export
//' @examples
//' \dontrun{
//' omicsds_handle <- omicsds::connect(workspace="~/feature-level-ws", array="18K_samples")
//' }
//'
// [[Rcpp::export]]
size_t connect(const std::string& workspace, const std::string& array) {
  Rcpp::Rcout << "Got OmicsDS" << std::endl;
  return OmicsDS::connect(workspace, array);
}

//' Disconnect from an OmicsDS workspace/array
//'
//' Disconnect from a given OmicsDS instance to release resources held by the
//' instance.
//'
//' @param handle OmicsDS instance representation for a workspace/array
//' @export
//' @examples
//' \dontrun{
//' omicsds_handle <- omicsds::connect(workspace="~/feature-level-ws", array="18K_samples")
//' omicsds::disconnect(omicsds_handle)
//' }
//'
// [[Rcpp::export]]
void disconnect(size_t handle) {
  OmicsDS::disconnect(handle);
}

class RFeatureProcessor {
 public:
  RFeatureProcessor(std::vector<std::string> features) {
    if (features.size()) {
      m_features_set_in_constr = true;
      for (auto i=0ul; i<features.size(); i++) {
        m_features.insert({features[i], i});
      }
    }
  }

  void process(const std::string& feature_id, uint64_t sample_id, float score) {
    // Rcpp::Rcout << "From R: id=" << feature_id << " sample_id=" << sample_id << " score=" << score << std::endl;
    if (!started) {
      first_sample_id = sample_id;
      started = true;
    }
    auto scores = m_feature_scores.find(sample_id);
    if (scores == m_feature_scores.end()) {
      // std::vector<float> values(m_features.size()?m_features.size():1, R_NaN);
      std::vector<float> values;
      if (m_features_set_in_constr) {
        values.resize(m_features.size());
      } else {
        values.reserve(1024); // Arbitrary value for now
      }
      m_feature_scores.insert({sample_id, std::move(values)});
      scores = m_feature_scores.find(sample_id);
    }
    assert(scores != m_feature_scores.end());
    auto feature_index = get_feature_index(feature_id);
    if (m_features_set_in_constr) {
      scores->second[feature_index]=score;
    } else {
      // Will be in the order received for the feature
      scores->second.push_back(score);
    }
  }

  Rcpp::DataFrame to_data_frame() {
    Rcpp::CharacterVector sample_names(m_feature_scores.size());
    Rcpp::List vector_list(m_feature_scores.size());

    uint64_t i = 0;
    for (std::pair<uint64_t, std::vector<float>> feature_score :  m_feature_scores) {
      sample_names[i] = std::to_string(feature_score.first);
      vector_list[i++] = feature_score.second;
    }

    Rcpp::DataFrame data_frame(vector_list);
    data_frame.attr("names") = sample_names;

    Rcpp::CharacterVector feature_names(m_features.size());
    i = 0;
    for (std::pair<std::string, size_t> feature :  m_features) {
      feature_names[i++] = feature.first;
    }
    data_frame.attr("row.names") = feature_names;

    return data_frame;
  }

 private:
  bool m_features_set_in_constr = false;
  std::unordered_map<std::string, size_t> m_features;
  uint64_t get_feature_index(const std::string& id) {
    auto found = m_features.find(id);
    if (found == m_features.end()) {
      m_features.insert({id, m_features.size()});
      found = m_features.find(id);
    }
    return found->second;
  }

  bool started = false;
  uint64_t first_sample_id;
  std::unordered_map<uint64_t, std::vector<float>> m_feature_scores;
};

//' Slice OmicsDS array by features/samples
//'
//' Get a slice as a DataFrame from an OmicsDS instance referenced by handle,
//' given features and a sample range. Features and sample range can be
//' NULL, in which case all features and/or samples are returned from
//' OmicsDS.
//'
//' @param handle OmicsDS instance representation for a workspace/array
//' @param features slice by list of features or NULL for all features
//' @param sample_range slice by sample range or NULL for all samples
//' @return R data.frame for the OmicsDS workspace array sliced by features/samples
//' @export
//' @examples
//' \dontrun{
//' omicsds_handle <- omicsds::connect(workspace="~/feature-level-ws", array="18K_samples")
//' df <- omicsds::query_features(handle=omicsds_handle, features=c("ENSG00000138190", "ENSG00000243485"), sample_range=c(0, 2))
//' df1 <- omicsds::query_features(handle=omicsds_handle, features=NULL, sample_range=c(0, 2))
//' df2 <- omicsds::query_features(handle=omicsds_handle, features=c("ENSG00000138190"), sample_range=NULL)
//' df3 <- omicsds::query_features(handle=omicsds_handle, features=NULL, sample_range=NULL)
//' }
// [[Rcpp::export]]
Rcpp::DataFrame query_features(size_t handle, Rcpp::Nullable<Rcpp::CharacterVector> features = R_NilValue, Rcpp::Nullable<Rcpp::List> sample_range = R_NilValue) {
  std::vector<std::string> feature_vector;
  if (features.isNotNull()) {
    feature_vector = Rcpp::as<std::vector<std::string>>(features);
  }

  std::array<int64_t, 2> range_array =  Rcpp::as<std::array<int64_t, 2>>(sample_range);

  RFeatureProcessor feature_processor(feature_vector);
  auto bound = std::bind(&RFeatureProcessor::process, std::ref(feature_processor), std::placeholders::_1, std::placeholders::_2,  std::placeholders::_3);

  OmicsDS::query_features(handle, feature_vector, range_array, bound);

  return feature_processor.to_data_frame();
}
