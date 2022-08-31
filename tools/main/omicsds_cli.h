/**
 * @file   omicsds_cli.h
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
 * Header file for CLI utilities
 */
#pragma once

#include <getopt.h>

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

/* Enum for the actions a user can undertake, along with a sentinel value for failure. */
enum ACTION {
  QUERY,
  IMPORT,
  UNKNOWN,
};

/* Maps user typeable strings to the appropriate action. */
static std::map<std::string_view, ACTION> const ACTION_MAP = {
    {"query", QUERY},
    {"import", IMPORT},
};

class LongOptions {
 public:
  /**
   * Adds the given option.
   */
  void add_option(option opt);

  /**
   * Return all the options that have been added.
   */
  const option* get_options();

  /**
   * Generate a getopt compatible optstring from the added options.
   */
  std::string optstring();

 private:
  std::vector<option> m_options;
};

/**
 * Return which action has been specified in argv.
 */
ACTION get_action(int argc, char* argv[]);

/**
 * Populate opt_map with the results of parsing agrv using the specified options. Returns true on
 * success or false if it encounters and unexpected option.
 */
bool parse_args(int argc, char* argv[], const option* long_options, const char* optstring,
                std::map<char, std::string_view>& opt_map);

/**
 * Get the value stored at key and store it in option, returning true on success and false
 * otherwise.
 */
bool get_option(const std::map<char, std::string_view>& opt_map, const char key,
                std::string_view& option);

/**
 * Callback class for generating a matrix file from feature level array
 */
class MatrixFileProcessor {
 public:
  MatrixFileProcessor(std::ostream* output_stream) : m_output_stream(output_stream) {}
  void process(const std::string& feature_id, uint64_t sample_id, float score);
  void set_inverse_sample_map(std::string_view sample_map_file);

 private:
  std::ostream* m_output_stream;
  bool m_first_row = true;
  bool m_first_entry = true;
  std::string m_prev_feature_id;
  std::vector<float> m_scores;
  std::shared_ptr<std::unordered_map<size_t, std::string>> m_inverse_sample_map = NULL;
};

/**
 * Main function for import logic
 */
int import_main(int argc, char* argv[], LongOptions long_options);

/**
 * Main function for query logic
 */
int query_main(int argc, char* argv[], LongOptions long_options);