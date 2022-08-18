/**
 * @file   cli_utils.cc
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
 * Implementation for CLI utilities
 */

#include "omicsds_cli.h"
#include "omicsds_logger.h"

ACTION get_action(int argc, char* argv[]) {
  if (argc < 2) {
    return ACTION::UNKNOWN;
  }
  try {
    return ACTION_MAP.at(std::string_view(argv[1]));
  } catch (std::out_of_range& ex) {
    return ACTION::UNKNOWN;
  }
}

bool parse_args(int argc, char* argv[], const option* long_options, const char* optstring,
                std::map<char, std::string_view>& opt_map) {
  int val = -1;
  while ((val = getopt_long(argc, argv, optstring, long_options, NULL)) != -1) {
    if (val == '?') {
      logger.warn("Found unexpected character {} while parsing arguments.", (char)optopt);
      return false;
    }
    if (optarg != NULL) {
      opt_map[val] = std::string_view(optarg);
    } else {
      opt_map[val] = std::string_view("");
    }
  }
  return true;
}

bool get_option(const std::map<char, std::string_view>& opt_map, const char key,
                std::string_view& option) {
  if (opt_map.count(key) == 1) {
    option = opt_map.at(key);
    return true;
  }
  return false;
}

void LongOptions::add_option(option opt) { m_options.push_back(opt); }

const option* LongOptions::get_options() {
  if (m_options.size() == 0) {
    return NULL;
  } else {
    return m_options.data();
  }
}

std::string LongOptions::optstring() {
  std::string optstring = "";
  char char_val[2] = {'\0', '\0'};
  for (auto it = m_options.begin(); it != m_options.end(); it++) {
    char_val[0] = it->val;
    optstring.append(char_val);
    if (it->has_arg == required_argument) optstring.append(":");
  }
  return optstring;
}
