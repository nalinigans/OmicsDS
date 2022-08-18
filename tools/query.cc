/**
 * The MIT License (MIT)
 * Copyright (c) 2022 Omics Data Automation, Inc.
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

#include "omicsds_cli.h"
#include "omicsds_export.h"

void print_query_usage() {
  std::cout << "Usage: omicsds query [options]\n"
            << "\nQuery options\n"
            << "\t \e[1m--workspace\e[0m, \e[1m-w\e[0m Path to workspace\n"
            << "\t \e[1m--array\e[0m, \e[1m-a\e[0m Name of array (should not "
               "include path to workspace)\n"
            << "\t \e[1m--generic\e[0m, \e[1m-g\e[0m Command to perform "
               "generic query. WIP. Output is probably not useful.\n"
            << "\t \e[1m--export-sam\e[0m, \e[1m-e\e[0m Command to export data "
               "from query range as sam files, one per sample. Should only be "
               "used on data ingested via --read-level\n";
}

int query_main(int argc, char* argv[], LongOptions long_options) {
  std::map<char, std::string_view> opt_map;
  long_options.add_option({"generic", no_argument, NULL, 'g'});
  long_options.add_option({"export-sam", no_argument, NULL, 'e'});
  if (!parse_args(argc, argv, long_options.get_options(), long_options.optstring().c_str(),
                  opt_map)) {
    print_query_usage();
    return -1;
  }

  std::string_view workspace;
  std::string_view array;
  if (!get_option(opt_map, 'w', workspace) || !get_option(opt_map, 'a', array)) {
    print_query_usage();
    return -1;
  }

  if (opt_map.count('g') == 1) {
    OmicsExporter r(workspace.data(), array.data());
    r.query();
  } else if (opt_map.count('e') == 1) {
    SamExporter s(workspace.data(), array.data());
    s.export_sams();
  } else {
    print_query_usage();
    return -1;
  }
  return 0;
}