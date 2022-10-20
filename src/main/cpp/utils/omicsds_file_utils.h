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

class OmicsDSTileDBUtils {
 public:
  template <typename... Args>
  static void check(int rc, const char* fmt, const Args&... args);
};

// Reading/Writing local/cloud files using TileDBUtils api
struct FileUtility : public OmicsDSTileDBUtils {
  // Constructor for reading, write functionality is static. File should exist, else
  // a OmicsDSException is thrown
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

  // returns true if path exists and as a file
  static bool is_file(const std::string& path);

  // returns true if path is a workspace
  static bool is_workspace(const std::string& workspace);

  // write string to file
  // returns tiledb return code
  static int write_file(const std::string& filename, const std::string& str,
                        const bool overwrite = false);

  // write buffer to file
  // returns tiledb return code
  static int write_file(const std::string& filename, const void* buffer, size_t length,
                        const bool overwrite = false);

  static inline std::string slashify(const std::string& path) {
    if (path.empty()) {
      return "/";
    } else if (path.back() != '/') {
      return path + '/';
    } else {
      return path;
    }
  }

  static inline std::string slashify(std::string_view path) { return slashify(std::string(path)); }

  static inline std::string unslashify(const std::string& path) {
    if (!path.empty() && path.back() == '/') {
      return path.substr(0, path.size() - 1);
    } else {
      return path;
    }
  }

  // TODO: replace append using variadic templates, so we can use any number of paths for appending.
  // Generally simple, but here we do not want to slashify the last path!
  static std::string append(const std::string& path1, const std::string& path2) {
    return slashify(path1) + path2;
  }

  static std::string append(std::string_view path1, std::string_view path2) {
    return slashify(path1) + std::string(path2);
  }

  static std::string append(const std::string& path1, const std::string& path2,
                            const std::string& path3) {
    return slashify(path1) + slashify(path2) + path3;
  }

  static std::string append(std::string_view path1, std::string_view path2,
                            std::string_view path3) {
    return slashify(path1) + slashify(path2) + std::string(path3);
  }

 private:
  size_t read_from_str_buffer(void* buffer, size_t chars_to_read);
};

// split str into tokens by sep
// similar to java/python split
std::vector<std::string> split(std::string str, const std::string& sep);
