/**
 * @file   omicsds_cli.h
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2022 Omics Data Automation, Inc.
 * @copyright Copyright (c) 2023 dātma, inc™
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
 * Header file for CLI Constants
 */
#pragma once

#include <getopt.h>

#include <array>
#include <map>
#include <string_view>

/* Enum for the actions a user can undertake, along with a sentinel value for failure. */
enum ACTION {
  CONFIGURE,
  CONSOLIDATE,
  QUERY,
  IMPORT,
  UNKNOWN,
};

/* Maps user typeable strings to the appropriate action. */
static std::map<std::string_view, ACTION> const ACTION_MAP = {
    {"configure", CONFIGURE},
    {"consolidate", CONSOLIDATE},
    {"query", QUERY},
    {"import", IMPORT},
};

/* Constants for valid command line arguments */
/* Shared options */
const char WORKSPACE = 'w';
const char ARRAY = 'a';
const char SAMPLE_MAP = 's';
static const std::array<const char, 3> SHARED_OPTIONS = {
    WORKSPACE,
    ARRAY,
    SAMPLE_MAP,
};

/* Import options */
const char READ_LEVEL = 'r';
const char INTERVAL_LEVEL = 'i';
const char FEATURE_LEVEL = 'f';
const char FILE_LIST = 'l';
const char MAPPING_FILE = 'm';
const char SAMPLE_MAJOR = 'p';
const char CONSOLIDATE_IMPORT = 'c';
static const std::array<const char, 7> IMPORT_OPTIONS = {
    READ_LEVEL,   INTERVAL_LEVEL, FEATURE_LEVEL,      FILE_LIST,
    MAPPING_FILE, SAMPLE_MAJOR,   CONSOLIDATE_IMPORT,
};

/* Query options */
const char GENERIC = 'g';
const char EXPORT_MATRIX = 'x';
const char EXPORT_SAM = 'e';
static const std::array<const char, 3> QUERY_OPTIONS = {
    GENERIC,
    EXPORT_MATRIX,
    EXPORT_SAM,
};

/* Long option mapping for CLI args */
static const std::map<char, option> OPTION_MAP = {
    {WORKSPACE, {"workspace", required_argument, NULL, WORKSPACE}},
    {ARRAY, {"array", required_argument, NULL, ARRAY}},
    {SAMPLE_MAP, {"sample-map", required_argument, NULL, SAMPLE_MAP}},
    {READ_LEVEL, {"read-level", no_argument, NULL, READ_LEVEL}},
    {INTERVAL_LEVEL, {"interval-level", no_argument, NULL, INTERVAL_LEVEL}},
    {FEATURE_LEVEL, {"feature-level", no_argument, NULL, FEATURE_LEVEL}},
    {FILE_LIST, {"file-list", required_argument, NULL, FILE_LIST}},
    {MAPPING_FILE, {"mapping-file", required_argument, NULL, MAPPING_FILE}},
    {SAMPLE_MAJOR, {"sample-major", no_argument, NULL, SAMPLE_MAJOR}},
    {CONSOLIDATE_IMPORT, {"consolidate", no_argument, NULL, CONSOLIDATE_IMPORT}},
    {GENERIC, {"generic", no_argument, NULL, GENERIC}},
    {EXPORT_MATRIX, {"export-matrix", no_argument, NULL, EXPORT_MATRIX}},
    {EXPORT_SAM, {"export-sam", no_argument, NULL, EXPORT_SAM}}};
