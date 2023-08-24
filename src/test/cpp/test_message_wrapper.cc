/**
 * src/test/cpp/test_message_wrapper.cc
 *
 * The MIT License (MIT)
 * Copyright (c) 2022 Omics Data Automation, Inc.
 * @copyright Copyright (C) 2023 dātma, inc™
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
 * Test OmicsDSMessageWrapper
 */
#include "catch.h"
#include "test_base.h"

#include "omicsds_array_metadata.h"
#include "omicsds_exception.h"
#include "omicsds_file_utils.h"
#include "omicsds_message_wrapper.h"

#include <memory>
#include <string>

void construct_message(const std::string& message_file,
                       std::shared_ptr<OmicsDSMessage<ArrayMetadata>>& message_pointer) {
  message_pointer =
      std::make_shared<OmicsDSMessage<ArrayMetadata>>(message_file, MessageFormat::JSON);
}

TEST_CASE_METHOD(TempDir, "test message wrapper", "[test_message_wrapper]") {
  std::string message_file = append("message_file");
  std::shared_ptr<OmicsDSMessage<ArrayMetadata>> message_pointer = nullptr;

  SECTION("Test missing file", "[test_message_wrapper load]") {
    REQUIRE_NOTHROW(construct_message(message_file, message_pointer));
    REQUIRE(!message_pointer->loaded_from_file());
  }
  SECTION("Test malformed file", "[test_message_wrapper load]") {
    FileUtility::write_file(message_file, "bad data");
    REQUIRE_THROWS_AS(construct_message(message_file, message_pointer), OmicsDSException);
  }
  SECTION("Test well formed file", "[test_message_wrapper load]") {
    FileUtility::write_file(message_file, "{\"extents\":[]}");
    REQUIRE_NOTHROW(construct_message(message_file, message_pointer));
    REQUIRE(message_pointer->loaded_from_file());
  }
}
