#include <catch2/catch.hpp>
#include "test_base.h"

#include "omicsds.h"

TEST_CASE("test version - sanity check", "[version]") {
  CHECK(!OmicsDS::version().empty());
}

TEST_CASE("test basic query", "[basic-feature-query]") {
  auto handle = OmicsDS::connect(std::string(OMICSDS_TEST_INPUTS)+"feature-level-ws", "array");
  CHECK(handle >= 0);
  std::vector<std::string> empty_features = {};
  std::array<int64_t, 2> sample_range = {0, 1000000};
  OmicsDS::query_features(handle, empty_features, sample_range);
  OmicsDS::disconnect(handle);
}
