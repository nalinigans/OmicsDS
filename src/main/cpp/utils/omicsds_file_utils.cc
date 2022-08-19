/**
 * @file   omicsds_file_utils.cc
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2022 Omics Data Automation, Inc.
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
 * This file implements the FileUtility struct
 */

#include <iostream>

#include "omicsds_file_utils.h"
#include "omicsds_logger.h"
#include "tiledb_utils.h"

FileUtility::FileUtility(const std::string& filename, size_t buffer_size) : filename(filename) {
  if (!TileDBUtils::is_file(filename)) {
    logger.error("Filename {} does not specify the path to a file.", filename);
  }
  ssize_t read_file_size = TileDBUtils::file_size(filename);
  if (read_file_size <= 0) {
    logger.warn("Read invalid size of {}, for file {}.", read_file_size, filename);
    read_file_size = 0;
  }
  file_size = read_file_size;
  this->buffer_size = std::min<size_t>(buffer_size, file_size);
  buffer = new char[(this->buffer_size)];
}

FileUtility::~FileUtility() { delete[] buffer; }

bool FileUtility::generalized_getline(std::string& retval) {
  retval = "";

  while (chars_read < file_size || str_buffer.size()) {
    size_t idx = str_buffer.find('\n');
    if (idx != std::string::npos) {
      retval = retval + str_buffer.substr(0, idx);  // exclude newline
      str_buffer.erase(0, idx + 1);                 // erase newline
      return true;
    }

    retval = retval + str_buffer;
    str_buffer.clear();

    size_t chars_to_read = std::min<size_t>(buffer_size, file_size - chars_read);

    if (chars_to_read) {
      TileDBUtils::read_file(filename, chars_read, buffer, chars_to_read);
      chars_read += chars_to_read;
    }

    str_buffer.insert(str_buffer.end(), buffer, buffer + chars_to_read);
  }
  // Last line of the file may be lacking a newline
  return retval != "";
}

int FileUtility::read_file(void* buffer, size_t chars_to_read) {
  size_t buf_position = 0;
  if (str_buffer.size() > 0) {
    buf_position = read_from_str_buffer(buffer, chars_to_read);
    chars_to_read -= buf_position;
  }
  int rcode =
      TileDBUtils::read_file(filename, chars_read, (char*)buffer + buf_position, chars_to_read);
  chars_read += chars_to_read;
  return rcode;
}

int FileUtility::write_file(std::string filename, const std::string& str, const bool overwrite) {
  return TileDBUtils::write_file(filename, str.c_str(), str.size(), overwrite);
}

int FileUtility::write_file(std::string filename, const void* buffer, size_t length,
                            const bool overwrite) {
  return TileDBUtils::write_file(filename, buffer, length, overwrite);
}

size_t FileUtility::read_from_str_buffer(void* buffer, size_t chars_to_read) {
  size_t readable_chars = std::min<size_t>(str_buffer.size(), chars_to_read);
  memcpy(buffer, str_buffer.c_str(), readable_chars);
  str_buffer.erase(0, readable_chars);
  return readable_chars;
}
