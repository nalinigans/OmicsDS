/**
 * @file test_import_config.cc
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2022 Omics Data Automation, Inc.
 * @copyright Copyright (C) 2023 dātma, inc™
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
 * Test the omicsds CLI import config generation
 */
#include "catch.h"

#include "omicsds_cli.h"

TEST_CASE("test import config", "[cli import]") {
  SECTION("Empty Map") {
    std::map<char, std::string_view> map;
    OmicsDSImportConfig config = generate_import_config(map);
    REQUIRE(!config.file_list.has_value());
    REQUIRE(!config.import_type.has_value());
    REQUIRE(!config.mapping_file.has_value());
    REQUIRE(!config.sample_major);
    REQUIRE(!config.sample_map.has_value());
  }
  SECTION("Full map") {
    std::string_view file_list = "my-file-list";
    std::string_view mapping_file = "my-mapping-file";
    std::string_view sample_map = "my-sample-map";
    std::map<char, std::string_view> map = {{FILE_LIST, file_list},
                                            {FEATURE_LEVEL, ""},
                                            {MAPPING_FILE, mapping_file},
                                            {SAMPLE_MAP, sample_map},
                                            {SAMPLE_MAJOR, ""}};
    OmicsDSImportConfig config = generate_import_config(map);
    REQUIRE((config.file_list && *config.file_list == file_list));
    REQUIRE((config.import_type && *config.import_type == OmicsDSImportType::FEATURE_IMPORT));
    REQUIRE((config.mapping_file && *config.mapping_file == mapping_file));
    REQUIRE(config.sample_major);
    REQUIRE((config.sample_map && *config.sample_map == sample_map));
  }
}
