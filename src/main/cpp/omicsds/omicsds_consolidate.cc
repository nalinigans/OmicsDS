/**
 * src/main/cpp/loader/omicsds_consolidate.cc
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
 * Implementation for OmicsDS workspace consolidation
 */

#include "omicsds_consolidate.h"
#include "omicsds_logger.h"

#include <tiledb.h>
#include <tiledb_constants.h>

OmicsDSConsolidate::OmicsDSConsolidate(std::string_view workspace, std::string_view array)
    : OmicsModule(workspace.data(), array.data()) {}

bool OmicsDSConsolidate::consolidate() {
  if (tiledb_array_consolidate(m_tiledb_ctx, logger.format("{}/{}", m_workspace, m_array).data()) !=
      TILEDB_OK) {
    logger.warn("Failed to consolidate array {} in workspace {}: {}", m_array, m_workspace,
                tiledb_errmsg);
    return false;
  }
  return true;
}
