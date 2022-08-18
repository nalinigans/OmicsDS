/**
 * @file tools/import.cc
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
 * Test the omicsds CLI functions
 */
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "omicsds_cli.h"
#include "omicsds_loader.h"
#include "omicsds_logger.h"

#define ARGC(arr) sizeof(arr) / sizeof(char*)

void check_option(const option opt1, const option opt2) {
  REQUIRE(opt1.name == opt2.name);
  REQUIRE(opt1.has_arg == opt2.has_arg);
  REQUIRE(opt1.flag == opt2.flag);
  REQUIRE(opt1.val == opt2.val);
}

TEST_CASE("test CLI", "[cli]") {
  SECTION("test action switch", "[cli action]") {
    SECTION("too few aruments", "[cli action error]") {
      char* argv[] = {(char*)"prog"};
      REQUIRE(get_action(ARGC(argv), argv) == ACTION::UNKNOWN);
    }
    SECTION("unknown action", "[cli action error]") {
      char* argv[] = {(char*)"prog", (char*)"gibberish"};
      REQUIRE(get_action(ARGC(argv), argv) == ACTION::UNKNOWN);
    }
    SECTION("query action", "[cli action error]") {
      char* argv[] = {(char*)"prog", (char*)"query"};
      REQUIRE(get_action(ARGC(argv), argv) == ACTION::QUERY);
    }
    SECTION("import action", "[cli action error]") {
      char* argv[] = {(char*)"prog", (char*)"import"};
      REQUIRE(get_action(ARGC(argv), argv) == ACTION::IMPORT);
    }
  }
  SECTION("test parse args", "[cli parse_args]") {
    std::string o_arg = "o-value";
    std::string p_arg = "p-value";
    option long_options[] = {
        {"option", required_argument, NULL, 'o'},
        {"parameter", required_argument, NULL, 'p'},
    };

    SECTION("test short option parser", "[cli parse_args short]") {
      char* argv[] = {
          (char*)"prog", (char*)"-o", o_arg.data(), (char*)"-p", p_arg.data(),
      };
      std::map<char, std::string_view> opt_map;
      optind = 1;
      REQUIRE(parse_args(ARGC(argv), argv, long_options, "o:p:", opt_map));
      REQUIRE(opt_map['o'] == o_arg);
      REQUIRE(opt_map['p'] == p_arg);
    }
    SECTION("test long option parser", "[cli parse_args short]") {
      char* argv[] = {
          (char*)"prog", (char*)"--option", o_arg.data(), (char*)"--parameter", p_arg.data(),
      };
      std::map<char, std::string_view> opt_map;
      optind = 1;
      REQUIRE(parse_args(ARGC(argv), argv, long_options, "o:p:", opt_map));
      REQUIRE(opt_map['o'] == o_arg);
      REQUIRE(opt_map['p'] == p_arg);
    }
    SECTION("test option parser error", "[cli parse_args error]") {
      char* argv[] = {(char*)"prog", (char*)"-u"};
      std::map<char, std::string_view> opt_map;
      optind = 1;
      REQUIRE(!parse_args(ARGC(argv), argv, long_options, "o:p:", opt_map));
    }
  }
  SECTION("Test get_option", "[cli get_option]") {
    std::map<char, std::string_view> opt_map = {
        {'w', "workspace"},
    };
    SECTION("get_option key not present", "[cli get_option error]") {
      std::string_view not_present = "not_present";
      REQUIRE(!get_option(opt_map, 'a', not_present));
      REQUIRE(not_present == std::string_view("not_present"));
    }
    SECTION("get_option key present", "[cli get_option]") {
      std::string_view present = "present";
      REQUIRE(get_option(opt_map, 'w', present));
      REQUIRE(present == std::string_view("workspace"));
    }
  }
  SECTION("test long_option wrapper", "[cli long_option]") {
    LongOptions lo;
    const option* options = NULL;
    options = lo.get_options();
    REQUIRE(options == NULL);
    REQUIRE(lo.optstring() == "");

    option opt1 = {"option", required_argument, NULL, 'o'};
    lo.add_option(opt1);
    option opt2 = {"parameter", required_argument, NULL, 'p'};
    lo.add_option(opt2);
    option opt3 = {"no-arg", no_argument, NULL, 'n'};
    lo.add_option(opt3);
    option opt4 = {"opt-arg", optional_argument, NULL, 't'};
    lo.add_option(opt4);

    options = lo.get_options();
    check_option(options[0], opt1);
    check_option(options[1], opt2);
    check_option(options[2], opt3);
    check_option(options[3], opt4);
    REQUIRE(lo.optstring() == "o:p:nt");
  }
}
