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
 * configure logic for OmicsDS CLI
 *
 **/
#include <iostream>

#include "omicsds.h"
#include "omicsds_cli.h"
#include "omicsds_configure.h"

void print_configure_usage() {
  std::cout << "Usage: omicsds configure [options]\n"
            << "\nconfigure options\n"
            << "\t \e[1m--workspace\e[0m, \e[1m-w\e[0m Path to workspace\n"
            << "\e[1mconfigure\e[0m can also take all command line arguments that can be provided "
               "to import. Run \e[1momicsds import\e[0m to see usage.\n";
}

int configure_main(int argc, char* argv[], LongOptions long_options) {
  std::map<char, std::string_view> opt_map;
  long_options.populate_configure_options();
  if (!parse_args(argc, argv, long_options.get_options(), long_options.optstring().c_str(),
                  opt_map)) {
    print_configure_usage();
    return -1;
  }

  std::string_view workspace;
  if (!get_option(opt_map, WORKSPACE, workspace)) {
    print_configure_usage();
    return -1;
  }

  OmicsDSImportConfig config = generate_import_config(opt_map);
  OmicsDSConfigure configurer = OmicsDSConfigure(workspace);
  configurer.update_import_config(config);
  return 0;
}
