/**
 * The MIT License (MIT)
 * Copyright (c) 2022 Omics Data Automation, Inc.
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
 * Query logic for OmicsDS CLI
 *
 **/
#include <iostream>

#include "omicsds.h"
#include "omicsds_cli.h"
#include "omicsds_export.h"
#include "omicsds_samplemap.h"

void print_query_usage() {
  std::cout << "Usage: omicsds query [options]\n"
            << "\nQuery options\n"
            << "\t \e[1m--workspace\e[0m, \e[1m-w\e[0m Path to workspace\n"
            << "\t \e[1m--array\e[0m, \e[1m-a\e[0m Name of array (should not "
               "include path to workspace)\n"
            << "\t \e[1m--generic\e[0m, \e[1m-g\e[0m Command to perform "
               "generic query. WIP. Output is probably not useful.\n"
            << "\t \e[1m--export-matrix\e[0m, \e[1m-m\e[0m Command to generate "
               "matrix file from array. Should only be used on data ingested via --feature-level.\n"
            << "\t \e[1m--sample-map\e[0m, \e[1m-s\e[0m If given, the matrix file output by "
               "providing the -m flag will have sample names rather than row id's.\n"
            << "\t \e[1m--export-sam\e[0m, \e[1m-e\e[0m Command to export data "
               "from query range as sam files, one per sample. Should only be "
               "used on data ingested via --read-level\n";
}

int query_main(int argc, char* argv[], LongOptions long_options) {
  std::map<char, std::string_view> opt_map;
  long_options.populate_query_options();
  if (!parse_args(argc, argv, long_options.get_options(), long_options.optstring().c_str(),
                  opt_map)) {
    print_query_usage();
    return -1;
  }

  std::string_view workspace;
  std::string_view array;
  if (!get_option(opt_map, WORKSPACE, workspace) || !get_option(opt_map, ARRAY, array)) {
    print_query_usage();
    return -1;
  }

  if (opt_map.count(EXPORT_MATRIX) == 1 || opt_map.count(GENERIC) == 1) {
    OmicsDSHandle handle = OmicsDS::connect(workspace.data(), array.data());
    std::array<int64_t, 2> sample_range = {0, std::numeric_limits<int64_t>::max()};
    std::vector<std::string> features = {};

    MatrixFileProcessor file_processor("/dev/stdout");
    feature_process_fn_t feature_processor;
    if (opt_map.count(EXPORT_MATRIX) == 1) {
      if (opt_map.count(SAMPLE_MAP) == 1) {
        file_processor.set_inverse_sample_map(opt_map.at(SAMPLE_MAP));
      }
      feature_processor =
          std::bind(&MatrixFileProcessor::process, std::ref(file_processor), std::placeholders::_1,
                    std::placeholders::_2, std::placeholders::_3);
    } else if (opt_map.count(GENERIC) == 1) {
      feature_processor = NULL;
    }
    OmicsDS::query_features(handle, features, sample_range, feature_processor);

    OmicsDS::disconnect(handle);
  } else if (opt_map.count(EXPORT_SAM) == 1) {
    SamExporter s(workspace.data(), array.data());
    s.export_sams();
  } else {
    print_query_usage();
    return -1;
  }
  return 0;
}
