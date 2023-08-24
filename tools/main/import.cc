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
 * Import logic for OmicsDS CLI
 *
 **/
#include <iostream>

#include "omicsds_cli.h"
#include "omicsds_consolidate.h"
#include "omicsds_loader.h"

void print_import_usage() {
  std::cout << "Usage: omicsds import [options]\n"
            << "\nImport options\n"
            << "\t \e[1m--workspace\e[0m, \e[1m-w\e[0m Path to workspace\n"
            << "\t \e[1m--array\e[0m, \e[1m-a\e[0m Name of array (should not "
               "include path to workspace)\n"
            << "\t one of the following "
               "\e[1m--read-level|--interval-level|--feature-level\e[0m should "
               "be specified for import\n"
            << "\t \e[1m--read-level\e[0m, \e[1m-r\e[0m Option to ingest read "
               "level related data (file list should contain SAM files)\n"
            << "\t \e[1m--interval-level\e[0m, \e[1m-i\e[0m Option to ingest "
               "interval level data (file list should contain Bed files)\n"
            << "\t \e[1m--feature-level\e[0m, \e[1m-f\e[0m Option to ingest "
               "feature level data (file list should contain Matrix files)\n\n"
            << "\t \e[1m--file-list\e[0m, \e[1m-l\e[0m Path to file containing "
               "paths to files to be ingested (one path per line)\n"
            << "\t \e[1m--sample-map\e[0m, \e[1m-s\e[0m Path to file "
               "containing information mapping between samples names and row "
               "indices\n\t\t\tin OmicsDS (each line is a sample name and an "
               "integer row number separated by a tab)\n"
            << "\t \e[1m--mapping-file\e[0m, \e[1m-m\e[0m Path to file "
               "containing information to map from contig/offset pair to "
               "flattened\n\t\t\tcoordinates. Currently supports fasta.fai "
               "(only needs first 3 columns: contig name, length,\n\t\t\tand "
               "starting index separated by tabs). Not needed for ingesting "
               "feature-level data\n"
            << "\t \e[1m--consolidate\e[0m, \e[1m-c\e[0m If provided, the array will be "
               "consolidated after import.\n";
}

int import_main(int argc, char* argv[], LongOptions long_options) {
  std::map<char, std::string_view> opt_map;
  long_options.populate_import_options();
  if (!parse_args(argc, argv, long_options.get_options(), long_options.optstring().c_str(),
                  opt_map)) {
    print_import_usage();
    return -1;
  }

  std::string_view workspace;
  std::string_view array;
  std::string_view file_list;
  std::string_view sample_map;
  if (!get_option(opt_map, WORKSPACE, workspace) || !get_option(opt_map, ARRAY, array)) {
    print_import_usage();
    return -1;
  }

  OmicsDSImportConfig import_config = generate_import_config(opt_map);
  // Scoping this block lets us make sure the loader has given up it's hold on the workspace
  // prior to attempting consolidation
  {
    std::shared_ptr<OmicsLoader> loader = get_loader(workspace, array, import_config);
    if (loader == NULL) {
      print_import_usage();
      return -1;
    }
    loader->initialize();
    loader->import();
  }

  if (opt_map.count(CONSOLIDATE_IMPORT) == 1) {
    OmicsDSConsolidate consolidator(workspace, array);
    if (!consolidator.consolidate()) {
      return -1;
    }
  }

  return 0;
}
