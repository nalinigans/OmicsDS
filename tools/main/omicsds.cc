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
 * Experimental OmicsDS CLI
 *
 **/

#ifdef USE_GPERFTOOLS
#include "gperftools/profiler.h"
#endif

#ifdef USE_GPERFTOOLS_HEAP
#include "gperftools/heap-profiler.h"
#endif

#include <iostream>

#include "omicsds_cli.h"

void print_action_usage() {
  std::cout << "Usage: omicsds <command> <arguments>\n\n"
            << "Commands:\n\n"
            << "\timport\timport data to an OmicDS workspace\n"
            << "\tquery\tquery from an OmicsDS workspace\n";
}

int main(int argc, char* argv[]) {
#ifdef USE_GPERFTOOLS
  std::cerr << "Profiling Started" << std::endl;
  ProfilerStart("omicsds_import.gperf.prof");
#endif
#ifdef USE_GPERFTOOLS_HEAP
  HeapProfilerStart("omicsds_import.gperf.heap");
#endif
  LongOptions long_options;
  long_options.add_option({"workspace", required_argument, NULL, 'w'});
  long_options.add_option({"array", required_argument, NULL, 'a'});
  switch (get_action(argc, argv)) {
    case ACTION::IMPORT:
      import_main(--argc, &argv[1], long_options);
      break;
    case ACTION::QUERY:
      query_main(--argc, &argv[1], long_options);
      break;
    case ACTION::UNKNOWN:
      print_action_usage();
      break;
  }
#ifdef USE_GPERFTOOLS_HEAP
  HeapProfilerStop();
#endif
#ifdef USE_GPERFTOOLS
  ProfilerStop();
#endif
}