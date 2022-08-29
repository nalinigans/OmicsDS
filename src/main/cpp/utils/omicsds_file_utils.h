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
 * File/Storage Utilities
 *
 **/

#pragma once

#include <string>
#include <vector>

#define SLASHIFY(path) (path.back() != '/' ? path + '/' : path)
#define UNSLASHIFY(path) (path.back() == '/' ? path.substr(0, path.length() - 2) : path)

// for reading/writing local/cloud files using TileDBUtils api
struct FileUtility {
  // only need to construct for reading, write functionality is static
  FileUtility(const std::string& filename, size_t buffer_size = 1024 * 1024 * 8);
  ~FileUtility();

  std::string filename;
  size_t file_size = 0;
  size_t chars_read = 0;
  size_t buffer_size;
  char* buffer;
  std::string str_buffer;

  // returns true if line was read
  // provides similar functionality to std::getline but also supports cloud files
  bool generalized_getline(std::string& retval);
  // read specified number of bytes from file,
  // should work with generalized_getline but not tested
  // returns tiledb return code
  int read_file(void* buffer, size_t chars_to_read);

  // write string to file
  // returns tiledb return code
  static int write_file(std::string filename, const std::string& str, const bool overwrite = false);

  // write buffer to file
  // returns tiledb return code
  static int write_file(std::string filename, const void* buffer, size_t length,
                        const bool overwrite = false);

 private:
  size_t read_from_str_buffer(void* buffer, size_t chars_to_read);
};

// split str into tokens by sep
// similar to java/python split
std::vector<std::string> split(std::string str, std::string sep);
