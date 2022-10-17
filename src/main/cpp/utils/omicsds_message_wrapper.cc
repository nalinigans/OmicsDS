/**
 * @file   omicsds_message_wrapper.cc
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
 * Implementation file for wrapper around protobuf messages to manage loading and saving
 */

#include "tiledb_constants.h"
#include "tiledb_utils.h"

#include "omicsds_array_metadata.pb.h"
#include "omicsds_file_utils.h"
#include "omicsds_import_config.pb.h"
#include "omicsds_logger.h"
#include "omicsds_message_wrapper.h"

#include <google/protobuf/util/json_util.h>

template <typename T>
OmicsDSMessage<T>::OmicsDSMessage(std::string_view path, MessageFormat format)
    : m_file_path(path), m_message(std::make_shared<T>()), m_format(format) {
  m_loaded_from_file = TileDBUtils::is_file(m_file_path) && parse_message();
  if (!m_loaded_from_file) {
    logger.info("Failed to load message from path {}.", m_file_path);
  }
}

template <typename T>
OmicsDSMessage<T>::~OmicsDSMessage() {
  if (!save_message()) {
    logger.error("Failed to save message to path {}.", m_file_path);
  }
}

template <typename T>
bool OmicsDSMessage<T>::loaded_from_file() {
  return m_loaded_from_file;
}

template <typename T>
std::shared_ptr<T> OmicsDSMessage<T>::message() {
  return m_message;
}

template <typename T>
bool OmicsDSMessage<T>::save_message() {
  std::string message_string;
  bool serialized = false;
  switch (m_format) {
    case MessageFormat::BINARY:
      serialized = m_message->SerializeToString(&message_string);
      break;
    case MessageFormat::JSON:
      google::protobuf::util::JsonPrintOptions print_options;
      serialized =
          google::protobuf::util::MessageToJsonString(*m_message, &message_string, print_options)
              .ok();
      break;
  }
  return serialized && FileUtility::write_file(m_file_path, message_string, true) == TILEDB_OK;
}

template <typename T>
bool OmicsDSMessage<T>::parse_message() {
  FileUtility reader = FileUtility(m_file_path);
  size_t size = reader.file_size;
  void* buf = new unsigned char[size];
  bool file_read = reader.read_file(buf, size) == TILEDB_OK;
  bool parsed = false;
  switch (m_format) {
    case MessageFormat::BINARY:
      parsed = m_message->ParseFromArray(buf, size);
      break;
    case MessageFormat::JSON:
      google::protobuf::util::JsonParseOptions parse_options;
      std::string json_str = std::string((char*)buf, size);
      parsed = google::protobuf::util::JsonStringToMessage(json_str, m_message.get(), parse_options)
                   .ok();
      break;
  }
  return file_read && parsed;
}

template class OmicsDSMessage<ArrayMetadata>;
template class OmicsDSMessage<ImportConfig>;