/**
 * @file src/test/cpp/test_omicsds_configure.h
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
 * Test OmicsDS configurer
 */

#include <catch2/catch.hpp>
#include "test_base.h"

#include "omicsds_configure.h"

TEST_CASE_METHOD(TempDir, "test omicsds configure", "[test_omicsds_configure]") {
  SECTION("Non-existent configure") {
    std::string workspace = append("empty-workspace");
    OmicsDSConfigure config(workspace);
    OmicsDSImportConfig import_config = config.get_import_config();

    REQUIRE(!import_config.file_list);
    REQUIRE(!import_config.import_type);
    REQUIRE(!import_config.mapping_file);
    REQUIRE(!import_config.sample_major);
    REQUIRE(!import_config.sample_map);
  }

  SECTION("Update configure") {
    std::string workspace = append("update-workspace");
    std::string file_list = "file-list";
    SECTION("Update existing config") {
      {  // Scope here is needed because config file is written on call to ~OmicsDSConfigure
        OmicsDSConfigure config(workspace);
        OmicsDSImportConfig import_config;
        import_config.file_list = std::make_optional<std::string>(file_list);
        config.update_import_config(import_config);

        OmicsDSImportConfig retreived_config = config.get_import_config();
        REQUIRE((retreived_config.file_list && *retreived_config.file_list == file_list));
      }
      SECTION("Read updated config") {
        OmicsDSConfigure config(workspace);
        OmicsDSImportConfig import_config = config.get_import_config();
        REQUIRE((import_config.file_list && *import_config.file_list == file_list));
      }
    }
  }
}
