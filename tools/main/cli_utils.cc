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
#include "omicsds_file_utils.h"
#include "omicsds_logger.h"
#include "omicsds_samplemap.h"

#include <iostream>
#include <string>

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

void LongOptions::populate_shared_options() {
  for (auto opt : SHARED_OPTIONS) {
    add_option(OPTION_MAP.at(opt));
  }
}

void LongOptions::populate_configure_options() { populate_import_options(); }

void LongOptions::populate_import_options() {
  for (auto opt : IMPORT_OPTIONS) {
    add_option(OPTION_MAP.at(opt));
  }
}

void LongOptions::populate_query_options() {
  for (auto opt : QUERY_OPTIONS) {
    add_option(OPTION_MAP.at(opt));
  }
}

MatrixFileProcessor::~MatrixFileProcessor() {
  write("\n");
  flush_buffer();
}

void MatrixFileProcessor::set_inverse_sample_map(std::string_view sample_map_file) {
  m_inverse_sample_map = SampleMap(sample_map_file.data()).invert_sample_map(true);
}

void MatrixFileProcessor::flush_buffer() {
  m_str_buf[m_buf_offset] = '\0';
  FileUtility::write_file(m_output_file, m_str_buf.data());
  m_buf_offset = 0;
}

void MatrixFileProcessor::write(const std::string& str) {
  if (str.length() + m_buf_offset >= BUFFER_SIZE - 1) {
    flush_buffer();
  }
  strncpy(m_str_buf.data() + m_buf_offset, str.data(), str.length());
  m_buf_offset += str.length();
}

void MatrixFileProcessor::process(const std::string& feature_id, uint64_t sample_id, float score) {
  if (m_first_entry) {
    write("SAMPLE");
    m_prev_feature_id = feature_id;
    m_first_entry = false;
  }
  if (m_prev_feature_id != feature_id) {
    if (m_first_row) {
      write(logger.format("\n{}", m_prev_feature_id));
      for (float score : m_scores) {
        write(logger.format("\t{:f}", score));
      }
      m_first_row = false;
      m_scores.clear();  // Free up fields now that we don't need them
      m_inverse_sample_map = NULL;
    }
    write(logger.format("\n{}", feature_id));
    m_prev_feature_id = feature_id;
  }
  if (m_first_row) {
    m_scores.emplace_back(score);
    if (m_inverse_sample_map != NULL) {
      write(logger.format("\t{}", m_inverse_sample_map->at(sample_id)));
    } else {
      write(logger.format("\t{}", sample_id));
    }
  } else {
    write(logger.format("\t{:f}", score));
  }
}

OmicsDSImportConfig generate_import_config(const std::map<char, std::string_view>& opt_map) {
  OmicsDSImportConfig import_config;
  if (opt_map.count(FEATURE_LEVEL) == 1) {
    import_config.import_type =
        std::make_optional<OmicsDSImportType>(OmicsDSImportType::FEATURE_IMPORT);
  } else if (opt_map.count(READ_LEVEL) == 1) {
    import_config.import_type =
        std::make_optional<OmicsDSImportType>(OmicsDSImportType::READ_IMPORT);
  } else if (opt_map.count(INTERVAL_LEVEL) == 1) {
    import_config.import_type =
        std::make_optional<OmicsDSImportType>(OmicsDSImportType::INTERVAL_IMPORT);
  }

  if (opt_map.count(FILE_LIST) == 1) {
    import_config.file_list = std::make_optional<std::string>(opt_map.at(FILE_LIST));
  }
  if (opt_map.count(MAPPING_FILE) == 1) {
    import_config.mapping_file = std::make_optional<std::string>(opt_map.at(MAPPING_FILE));
  }
  if (opt_map.count(SAMPLE_MAP) == 1) {
    import_config.sample_map = std::make_optional<std::string>(opt_map.at(SAMPLE_MAP));
  }
  if (opt_map.count(SAMPLE_MAJOR) == 1) {
    import_config.sample_major = true;
  }
  return import_config;
}
