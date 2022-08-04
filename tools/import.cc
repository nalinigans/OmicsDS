/**
 * The MIT License (MIT)
 * Copyright (c) 2022 Omics Data Automation, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * Experimental OmicsDS CLI
 *
 **/

#include <iostream>
#include <getopt.h>
#include "omicsds_loader.h"
#include "omicsds_export.h"
#include <chrono>
#include <thread>

#ifdef USE_GPERFTOOLS
#include "gperftools/profiler.h"
#endif

#ifdef USE_GPERFTOOLS_HEAP
#include "gperftools/heap-profiler.h"
#endif

enum ArgsEnum {
  ARGS_IDX_MAPPER
};

void print_usage() {
  std::cout << "Usage: omicsds_import [options]\n"
            << "where options include:\n"
            << "\t \e[1m--workspace\e[0m, \e[1m-w\e[0m Path to workspace\n"
            << "\t \e[1m--array\e[0m, \e[1m-a\e[0m Name of array (should not include path to workspace)\n"
            << "\nImport options\n"
            <<"\t one of the following \e[1m--read-level|--interval-level|--feature-level\e[0m should be specified for import\n"
            << "\t \e[1m--read-level\e[0m, \e[1m-r\e[0m Option to ingest read level related data (file list should contain SAM files)\n"
            << "\t \e[1m--interval-level\e[0m, \e[1m-i\e[0m Option to ingest interval level data (file list should contain Bed files)\n"
            << "\t \e[1m--feature-level\e[0m, \e[1m-f\e[0m Option to ingest feature level data (file list should contain Matrix files)\n\n"
            << "\t \e[1m--file-list\e[0m, \e[1m-l\e[0m Path to file containing paths to files to be ingested (one path per line)\n"
            << "\t \e[1m--sample-map\e[0m, \e[1m-s\e[0m Path to file containing information mapping between samples names and row indices\n\t\t\tin OmicsDS (each line is a sample name and an integer row number separated by a tab)\n"
            << "\t \e[1m--mapping-file\e[0m, \e[1m-m\e[0m Path to file containing information to map from contig/offset pair to flattened\n\t\t\tcoordinates. Currently supports fasta.fai (only needs first 3 columns: contig name, length,\n\t\t\tand starting index separated by tabs). Not needed for ingesting feature-level data\n"

            << "\t \e[1m--sample-major\e[0m Option to indicate that data from input files should be stored in sample major order on disk\n\t\t\t(cells for the same sample stored contiguously). Default behavior is position major\n"
            << "\nQuery options\n"
            << "\t \e[1m--generic-query\e[0m, \e[1m-g\e[0m Command to perform generic query. WIP. Output is probably not useful.\n"
            << "\t\t Optional\n"
            << "\t \e[1m--export-sam\e[0m Command to export data from query range as sam files, one per sample. Should only be used on data ingested via --read-level\n"
            << "\t\t Optional\n";
}

int main(int argc, char* argv[]) {
  char* cwd1 = getcwd(0, 0);

  enum options_enum { EXPORT_SAM, SAMPLE_MAJOR, GENE_MAPPING_FILE };

  static struct option long_options[] = {
    {"workspace",1,0,'w'},
    {"array",1,0,'a'},
    {"read-level",0,0,'r'},
    {"interval-level", 0, 0, 'i'},
    {"feature-level", 0, 0, 'f'},
    {"file-list",1,0,'l'},
    {"sample-map",1,0,'s'},
    {"mapping-file",1,0,'m'},
    {"generic-query", 0, 0, 'g'},
    {"export-sam", 0, 0, EXPORT_SAM},
    {"sample-major", 0, 0, SAMPLE_MAJOR},
  };

  std::string workspace = "";
  std::string array = "";
  std::string mapping_file = "";
  std::string file_list = "";
  std::string sample_map = "";
  bool read_level = false;
  bool interval_level = false;
  bool feature_level = false;
  bool generic_query = false;
  bool export_sam = false;
  bool position_major = true;

  int c;
  while ((c=getopt_long(argc, argv, "w:a:rifl:s:m:g", long_options, NULL)) >= 0) {
    switch (c) {
      case 'w':
        workspace = std::move(std::string(optarg));
        break;
      case 'a':
        array = std::move(std::string(optarg));
        break;
      case 'r':
        read_level = true;
        break;
      case 'i':
        interval_level = true;
        break;
      case 'f':
        feature_level = true;
        break;
      case 'l':
        file_list = std::move(std::string(optarg));
        break;
      case 's':
        sample_map = std::move(std::string(optarg));
        break;        
      case 'm':
        mapping_file = std::move(std::string(optarg));
        break;
      case 'g':
        generic_query = true;
        break;
      case EXPORT_SAM:
        export_sam = true;
        break;
      case SAMPLE_MAJOR:
        position_major = false;
        break;
      default:
        std::cerr << "Unknown command line argument " << char(c) << "\n";
        print_usage();
        return -1;
    }
  }

  // Validate Options
  if(workspace.empty()) {
    std::cerr << "Workspace required\n";
    print_usage();
    return -1;
  }
  if(array.empty()) {
    std::cerr << "Array required\n";
    print_usage();
    return -1;
  }
  if (!generic_query && !export_sam) {
    if (!read_level && !interval_level && !feature_level) {
      std::cerr << "One of (--read-level/-r) or (--interval-level/-i) or (--feature-level/-f) is required to be specified\n";
      return -1;
    }
  }
  if (read_level || interval_level || feature_level) {
    if(file_list.empty()) {
      std::cerr << "File list required\n";
      print_usage();
      return -1;
    }
    if(sample_map.empty()) {
      std::cerr << "Sample map required\n";
      print_usage();
      return -1;
    }
  }
  if (read_level || interval_level) {
    if(mapping_file.empty()) {
      std::cerr << "Mapping file required\n";
      print_usage();
      return -1;
    }
  }

#ifdef USE_GPERFTOOLS
  std::cout << "Profiling Started" << std::endl;
  ProfilerStart("omicsds_import.gperf.prof");
#endif
#ifdef USE_GPERFTOOLS_HEAP
  HeapProfilerStart("omicsds_import.gperf.heap");
#endif  
  
  if (read_level) {
    ReadCountLoader l(workspace, array, file_list, sample_map, mapping_file, position_major);
    l.initialize();
    l.import();
  } else if (interval_level) {
    TranscriptomicsLoader l(workspace, array, file_list, sample_map, mapping_file, "", position_major);
    l.initialize();
    l.import();
  } else if (feature_level) {
    MatrixLoader l(workspace, array, file_list, sample_map);
    l.initialize();
    l.import();
  }

  if(generic_query) {
    std::cout << "===================================== NORMAL QUERY =========================================" << std::endl;
    OmicsExporter r(workspace, array);
    r.query();
  }

  if(export_sam) {
    std::cout << "===================================== SAM QUERY =========================================" << std::endl;
    SamExporter s(workspace, array);
    s.export_sams();
  }

#ifdef USE_GPERFTOOLS_HEAP
    HeapProfilerStop();
#endif
#ifdef USE_GPERFTOOLS
    ProfilerStop();
#endif
}
