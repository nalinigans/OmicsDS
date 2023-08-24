/**
 * src/main/cpp/loader/omicsds_export.cc
 *
 * The MIT License (MIT)
 * Copyright (c) 2022 Omics Data Automation, Inc.
 * @copyright Copyright (C) 2023 dātma, inc™
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
 * Implementation of OmicsDS Export funtionality
 */

#include "omicsds_export.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <htslib/sam.h>

std::pair<std::vector<void*>, std::vector<size_t>> OmicsExporter::prepare_buffers() {
  m_buffers_vector.clear();
  std::vector<void*> pointers;
  std::vector<size_t> sizes;

  for (auto& [_, inf] : m_schema->attributes) {
    for (int i = 0; i < 1 + (inf.is_variable()); i++) {  // 1 buffer if fixed length, 2 if variable
      m_buffers_vector.emplace_back(m_buffer_size);
      pointers.push_back(m_buffers_vector.back().data());
      sizes.push_back(m_buffers_vector.back().size());
    }
  }
  // coords
  m_buffers_vector.emplace_back(m_buffer_size);
  pointers.push_back(m_buffers_vector.back().data());
  sizes.push_back(m_buffers_vector.back().size());

  return {pointers, sizes};
}

void OmicsExporter::query(std::array<int64_t, 2> sample_range,
                          std::array<int64_t, 2> position_range, process_function proc) {
  auto [pointers_vec, sizes_vec] = prepare_buffers();
  void** pointers = pointers_vec.data();
  size_t* sizes = sizes_vec.data();

  auto row_range = m_schema->position_major() ? position_range : sample_range;
  auto col_range = m_schema->position_major() ? sample_range : position_range;
  // int64_t subarray[] = { 0, std::numeric_limits<int64_t>::max(), 0, 0, 0,
  // std::numeric_limits<int64_t>::max() };

  int64_t subarray[] = {row_range[0],
                        row_range[1],
                        col_range[0],
                        col_range[1],
                        0,
                        std::numeric_limits<int64_t>::max()};

  m_array_storage->retrieve_by_cell(pointers_vec, sizes_vec, subarray,
                                    proc ? proc
                                         : std::bind(&OmicsExporter::process, this,
                                                     std::placeholders::_1, std::placeholders::_2));
}

void OmicsExporter::process(const std::array<uint64_t, 3>& coords,
                            const std::vector<OmicsFieldData>& data) {
  std::cout << "process " << coords[0] << ", " << coords[1] << ", " << coords[2] << std::endl;
  std::cout << data.size() << " fields" << std::endl;
  for (auto& a : data) {
    std::cout << "\t" << a.size() << " bytes" << std::endl;
  }
  std::cout << std::endl;
}

void OmicsExporter::check(const std::string& name, const OmicsFieldInfo& inf) {
  auto it = m_schema->attributes.find(name);
  if (it != m_schema->attributes.end()) {
    if (it->second == inf) {
      return;
    }
    std::cerr << "OmicsExporter error, field " << name << " in array does not match type "
              << inf.type_to_string() << " and/or length " << inf.length_to_string() << std::endl;
    std::cerr << "From schema: " << it->second.type_to_string() << ", "
              << it->second.length_to_string() << std::endl;
    exit(1);
  }
  std::cerr << "OmicsExporter error, field " << name << " is required" << std::endl;
  exit(1);
};

static std::string cigar_to_string(const uint32_t* cigar, size_t n_cigar) {
  const char cigar_codes[] = {'M', 'I', 'D', 'N', 'S', 'H', 'P', '=', 'X'};
  const uint32_t op_mask = 0b1111;
  const uint32_t len_mask = ~op_mask;

  std::stringstream ss;

  for (size_t i = 0; i < n_cigar; i++) {
    uint32_t op_idx = cigar[i] & op_mask;
    if (op_idx >= sizeof(cigar_codes)) {
      std::cerr << "Error, cigar is badly formed" << std::endl;
      exit(1);  // FIXME handle this better
    }
    char op = cigar_codes[op_idx];
    uint32_t len = (cigar[i] & len_mask) >> 4;
    ss << len << op;
  }
  return ss.str();
}

SamExporter::SamExporter(const std::string& workspace, const std::string& array)
    : OmicsExporter(workspace, array) {
  check("SAMPLE_NAME", OmicsFieldInfo(OmicsFieldInfo::OmicsFieldType::omics_char, -1));
  check("QNAME", OmicsFieldInfo(OmicsFieldInfo::OmicsFieldType::omics_char, -1));
  check("FLAG", OmicsFieldInfo(OmicsFieldInfo::OmicsFieldType::omics_uint16_t, 1));
  check("RNAME", OmicsFieldInfo(OmicsFieldInfo::OmicsFieldType::omics_char, -1));
  check("POS", OmicsFieldInfo(OmicsFieldInfo::OmicsFieldType::omics_int32_t, 1));
  check("MAPQ", OmicsFieldInfo(OmicsFieldInfo::OmicsFieldType::omics_uint8_t, 1));
  check("CIGAR", OmicsFieldInfo(OmicsFieldInfo::OmicsFieldType::omics_uint32_t, -1));
  check("RNEXT", OmicsFieldInfo(OmicsFieldInfo::OmicsFieldType::omics_int32_t, 1));
  check("PNEXT", OmicsFieldInfo(OmicsFieldInfo::OmicsFieldType::omics_int32_t, 1));
  check("TLEN", OmicsFieldInfo(OmicsFieldInfo::OmicsFieldType::omics_int32_t, 1));
  check("SEQ", OmicsFieldInfo(OmicsFieldInfo::OmicsFieldType::omics_char, -1));
  check("QUAL", OmicsFieldInfo(OmicsFieldInfo::OmicsFieldType::omics_char, -1));
}

void SamExporter::export_sams(std::array<int64_t, 2> sample_range,
                              std::array<int64_t, 2> position_range,
                              const std::string& output_prefix) {
  // open files
  std::map<int64_t, std::shared_ptr<std::ofstream>> files;

  process_function bound = std::bind(&SamExporter::sam_interface, this, files, output_prefix,
                                     std::placeholders::_1, std::placeholders::_2);
  query(sample_range, position_range, bound);
}

void SamExporter::sam_interface(std::map<int64_t, std::shared_ptr<std::ofstream>>& files,
                                const std::string& output_prefix,
                                const std::array<uint64_t, 3>& coords,
                                const std::vector<OmicsFieldData>& data) {
  int64_t row = coords[0];

  std::shared_ptr<std::ofstream> file;
  if (!files.count(row)) {
    files[row] = std::make_shared<std::ofstream>(output_prefix + std::to_string(row) + ".sam");
  }
  file = files[row];

  auto get_field = [&](const std::string& name) -> const OmicsFieldData& {  // TODO check bounds?
    return data[m_schema->index_of_attribute(name)];
  };

  // QNAME
  auto& qname_data = get_field("QNAME");
  std::string qname(qname_data.get_ptr<char>(), qname_data.size());
  *file << qname << "\t";

  // FLAG
  auto& flag_data = get_field("FLAG");
  *file << flag_data.get<uint16_t>() << "\t";

  // RNAME
  auto& rname_data = get_field("RNAME");
  std::string rname(rname_data.get_ptr<char>(), rname_data.size());
  *file << rname << "\t";

  // POS
  auto& pos_data = get_field("POS");
  *file << pos_data.get<int32_t>() << "\t";

  // MAPQ
  auto& mapq_data = get_field("MAPQ");
  *file << std::to_string(mapq_data.get<uint8_t>()) << "\t";

  // CIGAR
  auto& cigar_data = get_field("CIGAR");
  std::string cigar_string =
      cigar_to_string(cigar_data.get_ptr<uint32_t>(), cigar_data.typed_size<uint32_t>());
  *file << cigar_string << "\t";

  // RNEXT
  auto& rnext_data =
      get_field("RNEXT");  // FIXME should be a string but htslib turns it into a int32_t
  *file << rnext_data.get<int32_t>() << "\t";

  // PNEXT
  auto& pnext_data = get_field("PNEXT");
  *file << pnext_data.get<int32_t>() << "\t";

  // TLEN
  auto& tlen_data = get_field("TLEN");
  *file << tlen_data.get<int32_t>() << "\t";

  // SEQ
  auto& seq_data = get_field("SEQ");
  std::string seq(seq_data.get_ptr<char>(), seq_data.size());
  *file << seq << "\t";

  // QUAL
  auto& qual_data = get_field("QUAL");
  std::string qual(qual_data.get_ptr<char>(), qual_data.size());
  *file << qual;

  *file << std::endl;
}
