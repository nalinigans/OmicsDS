/**
 * src/main/cpp/storage/tiledb_storage.cc
 *
 * The MIT License (MIT)
 * Copyright (c) 2022 Omics Data Automation, Inc.
 * @copyright Copyright (c) 2023 dātma, inc™
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
 * TileDB implementation of OmicsDSArrayStorage
 */

#include "omicsds_tiledb_storage.h"
#include "omicsds_exception.h"
#include "omicsds_logger.h"
#include "omicsds_status.h"

#include "tiledb.h"
#include "tiledb_storage.h"
#include "tiledb_utils.h"

void TileDBArrayStorage::configure_workspace() {
  check(TileDBUtils::create_workspace(m_workspace, /*replace*/ true),
        "Could not create workspace={}", m_workspace);
}

void TileDBArrayStorage::initialize(bool write_mode, std::shared_ptr<OmicsSchema> omicsds_schema,
                                    bool overwrite_workspace, bool overwrite_array) {
  TileDB_Config tiledb_config = {};
  tiledb_config.home_ = m_workspace.c_str();

  check(tiledb_ctx_init(&m_tiledb_ctx, &tiledb_config), "Could not initialize tiledb with home={}",
        m_workspace);

  if (write_mode && omicsds_schema) {
    if (overwrite_workspace && is_workspace(m_tiledb_ctx, m_workspace)) {
      check(delete_dir(m_tiledb_ctx, m_workspace), "Could not delete existing tiledb workspace={}",
            m_workspace);
    }

    // Create a workspace
    if (!is_workspace(m_tiledb_ctx, m_workspace)) {
      check(tiledb_workspace_create(m_tiledb_ctx, m_workspace.c_str()),
            "Could not create tiledb workspace={}", m_workspace);
    }

    if (omicsds_schema) {
      if (overwrite_array && is_array(m_tiledb_ctx, m_array_path)) {
        check(delete_dir(m_tiledb_ctx, m_array_path), "Could not delete existing tiledb array={}",
              m_array_path);
      }

      if (!is_array(m_tiledb_ctx, m_array_path)) {
        // Prepare parameters for array schema
        std::vector<const char*> attributes_vec(omicsds_schema->attributes.size());
        std::vector<int32_t> cell_val_num_vec(omicsds_schema->attributes.size());
        std::vector<int32_t> types_vec(omicsds_schema->attributes.size() + 1);  // +1 for coords
        auto i = 0ul;
        for (auto const& attribute : omicsds_schema->attributes) {
          attributes_vec[i] = attribute.first.c_str();
          if (attribute.second.is_variable()) {
            cell_val_num_vec[i] = TILEDB_VAR_NUM;
          } else {
            cell_val_num_vec[i] = attribute.second.length;
          }
          types_vec[i] = to_field_type(attribute.second.type);
          i++;
        }
        types_vec[i] = TILEDB_INT64;  // coords

        const char** attributes = attributes_vec.data();
        const int* cell_val_num = cell_val_num_vec.data();
        const int* types = types_vec.data();

        int32_t order =
            TILEDB_ROW_MAJOR;  // different orders are implemented by reordering coordinates

        const char* dimensions[3];
        dimensions[2] = "LEVEL";
        if (omicsds_schema->position_major()) {
          dimensions[0] = "POSITION";
          dimensions[1] = "SAMPLE";
        } else {
          dimensions[0] = "SAMPLE";
          dimensions[1] = "POSITION";
        }

        // low/high limits per dimension
        int64_t domain[] = {
            0,
            std::numeric_limits<int64_t>::max(),  // 1st dimension limits (SAMPLE or POSITION
            // based on order)
            0,
            std::numeric_limits<int64_t>::max(),  // 2nd dimension limits
            0,
            std::numeric_limits<int16_t>::max()  // Level limits
        };

        std::vector<int32_t> compression_vec(omicsds_schema->attributes.size() + 1,
                                             TILEDB_GZIP);  // +1 for coordinates
        const int* compression = compression_vec.data();
        compression_vec[compression_vec.size() - 1] =
            compression_vec[compression_vec.size() - 1] |
            TILEDB_DELTA_ENCODE;  // Use delta encode for coordinates

        // TODO : Should get offset compression from protobuf configuration
        std::vector<int32_t> offsets_compression_vec(omicsds_schema->attributes.size(),
                                                     TILEDB_ZSTD + TILEDB_DELTA_ENCODE);
        const int* offsets_compression = offsets_compression_vec.data();

        // Set array schema
        TileDB_ArraySchema tiledb_array_schema;
        check(tiledb_array_set_schema(&tiledb_array_schema,               // Array schema struct
                                      m_array_path.c_str(),               // Array name
                                      attributes,                         // Attributes
                                      omicsds_schema->attributes.size(),  // Number of attributes
                                      1024,                               // Capacity
                                      order,                              // Cell order
                                      cell_val_num,         // Number of cell values per attribute
                                      compression,          // Compression
                                      NULL,                 // Compression level - Use defaults
                                      offsets_compression,  // Offsets compression
                                      NULL,                 // Offsets compression level
                                      0,                    // Sparse array
                                      dimensions,           // Dimensions
                                      3,                    // Number of dimensions
                                      domain,               // Domain
                                      6 * sizeof(int64_t),  // Domain length in bytes
                                      NULL,   // Tile extents (no regular tiles defined)
                                      0,      // Tile extents in bytes
                                      order,  // Tile order
                                      types   // Types
                                      ),
              "Could not set TileDB array schema for array={}", m_array_path);

        // Create array
        check(tiledb_array_create(m_tiledb_ctx, &tiledb_array_schema),
              "Could not create TileDB array={}", m_array_path);

        // Free array schema
        check(tiledb_array_free_schema(&tiledb_array_schema),
              "Could not free TileDB schema for array={}", m_array_path);
      }
    }
  }
  open_array(write_mode);
}

void TileDBArrayStorage::open_array(bool write_mode) {
  TileDB_Array* tiledb_array = nullptr;
  // Initialize array
  check(tiledb_array_init(m_tiledb_ctx,                                         // Context
                          &tiledb_array,                                        // Array object
                          m_array_path.c_str(),                                 // Array name
                          write_mode ? TILEDB_ARRAY_WRITE : TILEDB_ARRAY_READ,  // Mode
                          NULL,                                                 // Entire domain
                          NULL,                                                 // All attributes
                          0),  // Number of attributes
        "Could not initialize TileDB array={}", m_array_path);
  m_tiledb_array = tiledb_array;
}

// TODO: This should only be invoked in write mode. Add check!!
void TileDBArrayStorage::reopen_array() {
  if (m_tiledb_array) {
    check(tiledb_array_finalize(m_tiledb_array), "Could not finalize TileDB array={}",
          m_array_path);
  };
  open_array(true);
}

void TileDBArrayStorage::finalize() {
  // Finalize array
  if (m_tiledb_array) {
    check(tiledb_array_finalize(m_tiledb_array), "Could not finalize TileDB array={}",
          m_array_path);
  }
  m_tiledb_array = nullptr;

  // Finalize context
  if (m_tiledb_ctx) {
    check(tiledb_ctx_finalize(m_tiledb_ctx), "Could not finalize TileDB context for workspace={}",
          m_workspace);
  }
  m_tiledb_ctx = nullptr;
}

int TileDBArrayStorage::store(std::vector<void*>& buffers, std::vector<size_t>& buffer_sizes) {
  check(tiledb_array_write(m_tiledb_array, const_cast<const void**>(buffers.data()),
                           buffer_sizes.data()),
        "Could not store from buffers into TileDB for array={}", m_array_path);
  return OMICSDS_OK;
}

int TileDBArrayStorage::retrieve(std::vector<void*>& buffers, std::vector<size_t>& buffer_sizes) {
  check(tiledb_array_read(m_tiledb_array, buffers.data(), buffer_sizes.data()),
        "Could not retrieve into buffers from TileDB for array={}", m_array_path);
  return OMICSDS_OK;
}

std::unordered_map<int, size_t> tiledb_type_size = std::unordered_map<int, size_t>{
    {TILEDB_CHAR, 1},  {TILEDB_UINT8, 1},   {TILEDB_INT8, 1},   {TILEDB_UINT16, 2},
    {TILEDB_INT16, 2}, {TILEDB_UINT32, 4},  {TILEDB_INT32, 4},  {TILEDB_UINT64, 8},
    {TILEDB_INT64, 8}, {TILEDB_FLOAT32, 4}, {TILEDB_FLOAT64, 8}};

int TileDBArrayStorage::retrieve_by_cell(std::vector<void*>& buffers,
                                         std::vector<size_t>& buffer_sizes, int64_t* subarray,
                                         process_cell_t processor) {
  TileDB_ArraySchema tiledb_array_schema = {};
  check(tiledb_array_get_schema(m_tiledb_array, &tiledb_array_schema),
        "Could not get TileDB schema for array={}", m_array_path);

  check(tiledb_array_finalize(m_tiledb_array), "Failed to finalize array path={}", m_array_path);

  TileDB_ArrayIterator* tiledb_array_it;
  check(tiledb_array_iterator_init(m_tiledb_ctx,          // Context
                                   &tiledb_array_it,      // Array iterator
                                   m_array_path.c_str(),  // Array name
                                   TILEDB_ARRAY_READ,     // Mode
                                   subarray,              // Constrain in subarray
                                   NULL,                  // All attributes
                                   0,                     // Number of attributes
                                   buffers.data(),        // Buffers used internally
                                   buffer_sizes.data()),  // Buffer sizes
        "Could not initialize TileDB iterator for array={}", m_array_path);

  // Set up field data and presize the non-variable sized data fields for callback
  auto attributes = tiledb_array_schema.attribute_num_;
  std::vector<OmicsFieldData> data(attributes);
  for (auto i = 0; i < attributes; i++) {
    if (tiledb_array_schema.cell_val_num_[i] != TILEDB_VAR_NUM) {
      data[i].data.resize(tiledb_array_schema.cell_val_num_[i] *
                          tiledb_type_size[tiledb_array_schema.types_[i]]);
    }
  }
  std::array<uint64_t, 3> coords;

  const uint8_t* a1_v;
  size_t a1_size = 0;
  while (!tiledb_array_iterator_end(tiledb_array_it)) {
    for (auto i = 0; i < attributes; i++) {
      // Get value
      check(tiledb_array_iterator_get_value(
                tiledb_array_it,      // Array iterator
                i,                    // Attribute id
                (const void**)&a1_v,  // Value
                &a1_size),            // Value size (useful in variable-sized attributes);
            "Failed to read value from TileDB iterator for array={}", m_array_path);

      if (tiledb_array_schema.cell_val_num_[i] == TILEDB_VAR_NUM) {
        data[i].data.resize(a1_size);
      }
      memcpy(data[i].data.data(), a1_v, a1_size);
    }

    uint64_t* coords_ptr;
    check(tiledb_array_iterator_get_value(
              tiledb_array_it,            // Array iterator
              attributes,                 // Attribute id for coordinates
              (const void**)&coords_ptr,  // Value
              &a1_size),                  // Value size (useful in variable-sized attributes)
          "Failed to get coords value from TileDB iterator for array={}", m_array_path);

    coords = {coords_ptr[0], coords_ptr[1], coords_ptr[2]};
    if (strncmp(tiledb_array_schema.dimensions_[0], "POSITION", 8) == 0) {
      std::swap(coords[0], coords[1]);
    }

    processor(coords, data);

    // Advance iterator
    check(tiledb_array_iterator_next(tiledb_array_it),
          "Could not advance TileDB iterator for array={}", m_array_path);
  }

  check(tiledb_array_iterator_finalize(tiledb_array_it),
        "Failed to finalize TileDB array iterator for array={}", m_array_path);

  open_array(false);

  return OMICSDS_OK;
}

int TileDBArrayStorage::consolidate() {
  check(tiledb_array_consolidate(m_tiledb_ctx, m_array_path.c_str()),
        "Failed to consolidate TileDB array {}", m_array_path);
  return OMICSDS_OK;
}

int TileDBArrayStorage::to_field_type(OmicsFieldInfo::OmicsFieldType omics_type) {
  switch (omics_type) {
    case OmicsFieldInfo::omics_char:
      return TILEDB_CHAR;
    case OmicsFieldInfo::omics_uint8_t:
      return TILEDB_UINT8;
    case OmicsFieldInfo::omics_int8_t:
      return TILEDB_INT8;
    case OmicsFieldInfo::omics_uint16_t:
      return TILEDB_UINT16;
    case OmicsFieldInfo::omics_int16_t:
      return TILEDB_INT16;
    case OmicsFieldInfo::omics_uint32_t:
      return TILEDB_UINT32;
    case OmicsFieldInfo::omics_int32_t:
      return TILEDB_INT32;
    case OmicsFieldInfo::omics_uint64_t:
      return TILEDB_UINT64;
    case OmicsFieldInfo::omics_int64_t:
      return TILEDB_INT64;
    case OmicsFieldInfo::omics_float_t:
      return TILEDB_FLOAT32;
  }
  return TILEDB_CHAR;
}
