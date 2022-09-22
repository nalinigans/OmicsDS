/**
 * @file src/test/cpp/test_encoder.h
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
 * Test gtf ids encoding and decoding
 */

#include <catch2/catch.hpp>
#include "test_base.h"

#include "omicsds_array_metadata.pb.h"
#include "omicsds_loader.h"

TEST_CASE_METHOD(TempDir, "test MatrixLoader", "[MatrixLoader]") {
  std::string file_list = append("matrix-file-list");
  std::string matrix_file =
      std::string(std::string(OMICSDS_TEST_INPUTS) + "OmicsDSTests/test_matrix.sorted");
  FileUtility::write_file(file_list, matrix_file, true);
  std::string sample_map = std::string(std::string(OMICSDS_TEST_INPUTS) + "OmicsDSTests/small_map");

  SECTION("test extent expansion", "[MatrixLoader extent") {
    std::string workspace = append("extent-workspace");
    MatrixLoader ml = MatrixLoader(workspace, "array", file_list, sample_map);
    REQUIRE(ml.get_extent(Dimension::SAMPLE).first == -1ul);
    REQUIRE(ml.get_extent(Dimension::SAMPLE).second == 0ul);
    REQUIRE(ml.get_extent(Dimension::FEATURE).first == -1ul);
    REQUIRE(ml.get_extent(Dimension::FEATURE).second == 0ul);
    ml.expand_extent(Dimension::SAMPLE, 1);
    REQUIRE(ml.get_extent(Dimension::SAMPLE).first == 1ul);
    REQUIRE(ml.get_extent(Dimension::SAMPLE).second == 1ul);
    REQUIRE(ml.get_extent(Dimension::FEATURE).first == -1ul);
    REQUIRE(ml.get_extent(Dimension::FEATURE).second == 0ul);
    ml.expand_extent(Dimension::FEATURE, 1);
    REQUIRE(ml.get_extent(Dimension::SAMPLE).first == 1ul);
    REQUIRE(ml.get_extent(Dimension::SAMPLE).second == 1ul);
    REQUIRE(ml.get_extent(Dimension::FEATURE).first == 1ul);
    REQUIRE(ml.get_extent(Dimension::FEATURE).second == 1ul);
    ml.expand_extent(Dimension::SAMPLE, 2);
    REQUIRE(ml.get_extent(Dimension::SAMPLE).first == 1ul);
    REQUIRE(ml.get_extent(Dimension::SAMPLE).second == 2ul);
    REQUIRE(ml.get_extent(Dimension::FEATURE).first == 1ul);
    REQUIRE(ml.get_extent(Dimension::FEATURE).second == 1ul);

    ml.initialize();  // The destructors that will be invoked expect initialization to have occured
  }

  SECTION("test import extents", "[MatrixLoader extents import]") {
    std::string workspace = append("import-workspace");
    MatrixLoader ml = MatrixLoader(workspace, "array", file_list, sample_map);

    ml.initialize();
    ml.import();
    REQUIRE(ml.get_extent(Dimension::SAMPLE).first == 0ul);
    REQUIRE(ml.get_extent(Dimension::SAMPLE).second == 303ul);
    REQUIRE(ml.get_extent(Dimension::FEATURE).first == 281474976848846ul);
    REQUIRE(ml.get_extent(Dimension::FEATURE).second == 281474976954141ul);
  }

  SECTION("test protobuf extents") {
    std::string workspace = append("protobuf-workspace");
    {
      MatrixLoader ml = MatrixLoader(workspace, "array", file_list, sample_map);
      ml.initialize();
      ml.import();
    }

    OmicsDSArrayMetadata metadata = OmicsDSArrayMetadata(workspace + "/array/metadata");
    REQUIRE(metadata.get_extent(Dimension::SAMPLE).first == 0ul);
    REQUIRE(metadata.get_extent(Dimension::SAMPLE).second == 303ul);
    REQUIRE(metadata.get_extent(Dimension::FEATURE).first == 281474976848846ul);
    REQUIRE(metadata.get_extent(Dimension::FEATURE).second == 281474976954141ul);
  }
}
