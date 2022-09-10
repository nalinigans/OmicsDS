/**
 * @file src/test/cpp/test_api.cc
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2022 Omics Data Automation, Inc.
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
 * Test the OmicsDS api class
 */

#include <catch2/catch.hpp>
#include "test_base.h"

#include "omicsds.h"

#include <stdlib.h>
#include <iostream>

TEST_CASE("test version - sanity check", "[version]") { CHECK(!OmicsDS::version().empty()); }

class CountCells {
 public:
  void process(const std::string& feature_id, uint64_t sample_id, float score) { m_cells++; }
  uint64_t m_cells = 0;
};

class CheckCells {
 public:
  typedef struct test_cell_t {
    std::string m_feature_id;
    std::uint64_t m_sample_id;
    float m_score;
    test_cell_t(const std::string& feature_id, uint64_t sample_id, float score)
        : m_feature_id(feature_id), m_sample_id(sample_id), m_score(score) {}
  } test_cell_t;
  void process(const std::string& feature_id, uint64_t sample_id, float score) {
    m_cells.push_back(test_cell_t(feature_id, sample_id, score));
  }
  std::vector<test_cell_t> m_cells;
};

TEST_CASE("test basic query", "[basic-feature-query]") {
  auto handle = OmicsDS::connect(std::string(OMICSDS_TEST_INPUTS) + "feature-level-ws", "array");
  CHECK(handle >= 0);

  std::vector<std::string> empty_features = {};
  std::array<int64_t, 2> sample_range = {0, 1000000};

  SECTION("With empty features default") {
    OmicsDS::query_features(handle, empty_features, sample_range);
  }

  SECTION("With empty features") {
    CountCells count;
    auto bound = std::bind(&CountCells::process, std::ref(count), std::placeholders::_1,
                           std::placeholders::_2, std::placeholders::_3);
    OmicsDS::query_features(handle, empty_features, sample_range, bound);
    CHECK(count.m_cells == 608);
  }

  std::array<int64_t, 2> sub_sample_range = {5, 9};
  SECTION("With empty features and sub samples") {
    CountCells count;
    auto bound = std::bind(&CountCells::process, std::ref(count), std::placeholders::_1,
                           std::placeholders::_2, std::placeholders::_3);
    OmicsDS::query_features(handle, empty_features, sub_sample_range, bound);
    CHECK(count.m_cells == 10);
  }

  std::vector<std::string> one_feature = {"ENSG00000243485"};
  SECTION("With one feature and sub samples") {
    CountCells count;
    auto bound = std::bind(&CountCells::process, std::ref(count), std::placeholders::_1,
                           std::placeholders::_2, std::placeholders::_3);
    OmicsDS::query_features(handle, one_feature, sub_sample_range, bound);
    CHECK(count.m_cells == 5);
  }

  SECTION("With one feature, check various sub-sample ranges") {
    for (auto i = 0; i < 152; i++) {
      sub_sample_range[0] = i;
      sub_sample_range[1] = 303 - i;
      CheckCells check;
      auto bound = std::bind(&CheckCells::process, std::ref(check), std::placeholders::_1,
                             std::placeholders::_2, std::placeholders::_3);
      OmicsDS::query_features(handle, one_feature, sub_sample_range, bound);
      CHECK(check.m_cells.size() == (304 - 2 * i));
    }

    SECTION("With one feature, out of range sub-sample") {
      sub_sample_range[0] = 305;
      sub_sample_range[1] = 400;
      CheckCells check;
      auto bound = std::bind(&CheckCells::process, std::ref(check), std::placeholders::_1,
                             std::placeholders::_2, std::placeholders::_3);
      OmicsDS::query_features(handle, one_feature, sub_sample_range, bound);
      CHECK(check.m_cells.size() == 0);
    }
  }

  OmicsDS::disconnect(handle);

  // This is a matrix of 18k samples with 1000 features imported into 18k-1000-ws
  // Querying a subset of 20 samples for one feature should return 20 cells.
  SECTION("18k matrix") {
    handle = OmicsDS::connect(std::string(OMICSDS_TEST_INPUTS) + "18k-1000-ws", "array");
    CheckCells check;
    sub_sample_range[0] = 83;
    sub_sample_range[1] = 102;
    std::vector<std::string> one_feature = {"ENSG00000002834"};
    auto bound = std::bind(&CheckCells::process, std::ref(check), std::placeholders::_1,
                           std::placeholders::_2, std::placeholders::_3);
    OmicsDS::query_features(handle, one_feature, sub_sample_range, bound);
    CHECK(check.m_cells.size() == 20);
    OmicsDS::disconnect(handle);
  }
}
