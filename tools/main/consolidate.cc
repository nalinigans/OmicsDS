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
 * consolidate logic for OmicsDS CLI
 *
 **/
#include <iostream>

#include "omicsds.h"
#include "omicsds_cli.h"
#include "omicsds_consolidate.h"

void print_consolidate_usage() {
  std::cout << "Usage: omicsds consolidate [options]\n"
            << "\nconsolidate options\n"
            << "\t \e[1m--workspace\e[0m, \e[1m-w\e[0m Path to workspace\n"
            << "\t \e[1m--array\e[0m, \e[1m-a\e[0m Name of array (should not "
               "include path to workspace)\n";
}

int consolidate_main(int argc, char* argv[], LongOptions long_options) {
  std::map<char, std::string_view> opt_map;
  if (!parse_args(argc, argv, long_options.get_options(), long_options.optstring().c_str(),
                  opt_map)) {
    print_consolidate_usage();
    return -1;
  }

  std::string_view workspace;
  std::string_view array;
  if (!get_option(opt_map, WORKSPACE, workspace) || !get_option(opt_map, ARRAY, array)) {
    print_consolidate_usage();
    return -1;
  }

  OmicsDSConsolidate consolidator = OmicsDSConsolidate(workspace, array);
  if (consolidator.consolidate()) {
    return 0;
  } else {
    return -1;
  }
}
