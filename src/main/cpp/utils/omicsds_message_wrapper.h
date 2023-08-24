/**
 * @file   omicsds_message_wrapper.h
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2022 Omics Data Automation, Inc.
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
 * @section DESCRIPTION
 *
 * Header file for wrapper around protobuf messages to manage loading and saving
 */

#pragma once

#include <memory>
#include <string>
#include <string_view>

enum MessageFormat {
  BINARY,
  JSON,
};

template <class T>
class OmicsDSMessage {
 public:
  OmicsDSMessage(std::string_view path, MessageFormat format = MessageFormat::BINARY);
  ~OmicsDSMessage();

  /**
   * Whether or not the wrapped message was loaded from a file.
   */
  bool loaded_from_file();

  /**
   * Returns the underlying message.
   */
  std::shared_ptr<T> message();

 private:
  void save_message();
  void parse_message();
  std::string m_file_path;
  std::shared_ptr<T> m_message;
  bool m_loaded_from_file = false;
  MessageFormat m_format;
};
