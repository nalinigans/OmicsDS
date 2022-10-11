/**
 * src/main/cpp/loader/omicsds_configure.cc
 *
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
 * Implementation for OmicsDS workspace configuration
 */

#include "omicsds_configure.h"
#include "omicsds_logger.h"

#include "omicsds_import_config.pb.h"

OmicsDSConfigure::OmicsDSConfigure(std::string_view workspace)
    : OmicsModule(workspace.data(), ""),
      m_import_config(std::make_shared<OmicsDSMessage<ImportConfig>>(
          logger.format("{}/import_config", m_workspace), MessageFormat::JSON)) {}

void OmicsDSConfigure::update_import_config(const OmicsDSImportConfig& config) {
  std::shared_ptr<ImportConfig> import_config = m_import_config->message();

  // Import type
  if (config.import_type) {
    switch (*config.import_type) {
      case OmicsDSImportType::FEATURE_IMPORT:
        import_config->set_import_type(ImportType::FEATURE_LEVEL);
        break;
      case OmicsDSImportType::INTERVAL_IMPORT:
        import_config->set_import_type(ImportType::INTERVAL_LEVEL);
        break;
      case OmicsDSImportType::READ_IMPORT:
        import_config->set_import_type(ImportType::READ_LEVEL);
        break;
    }
  }

  if (config.file_list) {
    import_config->set_file_list(*config.file_list);
  }

  if (config.sample_map) {
    import_config->set_sample_map(*config.sample_map);
  }

  if (config.mapping_file) {
    import_config->set_mapping_file(*config.mapping_file);
  }

  if (config.sample_major) {
    import_config->set_sample_major(config.sample_major);
  }
}

OmicsDSImportConfig OmicsDSConfigure::get_import_config() {
  std::shared_ptr<ImportConfig> internal_import_config = m_import_config->message();

  OmicsDSImportConfig import_config;

  if (internal_import_config->has_import_type()) {
    OmicsDSImportType import_type;
    switch (internal_import_config->import_type()) {
      case ImportType::READ_LEVEL:
        import_type = OmicsDSImportType::READ_IMPORT;
        break;
      case ImportType::FEATURE_LEVEL:
        import_type = OmicsDSImportType::FEATURE_IMPORT;
        break;
      case ImportType::INTERVAL_LEVEL:
        import_type = OmicsDSImportType::INTERVAL_IMPORT;
        break;
    }
    import_config.import_type = std::make_optional<OmicsDSImportType>(import_type);
  }
  if (internal_import_config->has_file_list()) {
    import_config.file_list = std::make_optional<std::string>(internal_import_config->file_list());
  }
  if (internal_import_config->has_sample_map()) {
    import_config.sample_map =
        std::make_optional<std::string>(internal_import_config->sample_map());
  }
  if (internal_import_config->has_mapping_file()) {
    import_config.mapping_file =
        std::make_optional<std::string>(internal_import_config->mapping_file());
  }
  if (internal_import_config->has_sample_major()) {
    import_config.sample_major = internal_import_config->sample_major();
  }

  return import_config;
}
