/**
 * src/main/cpp/omicsds/omicsds_loader.cc
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
 * Implementation for supported Omics loaders
 */

#include "omicsds_loader.h"

#include "omicsds_configure.h"
#include "omicsds_encoder.h"
#include "omicsds_export.h"
#include "omicsds_file_utils.h"
#include "omicsds_logger.h"

#include "omicsds_array_metadata.pb.h"

#include <tuple>

void read_sam_file(std::string filename) {
  std::cerr << "SAM file is " << filename << std::endl;

  samFile* fp_in = hts_open(filename.c_str(), "r");  // open bam file
  bam_hdr_t* bamHdr = sam_hdr_read(fp_in);           // read header
  bam1_t* aln = bam_init1();                         // initialize an alignment

  if (!bamHdr) {
    std::cerr << "header is null" << std::endl;
  } else {
    std::cerr << "header is NOT null" << std::endl;
  }

  // header parse
  // uint32_t *tar = bamHdr->text ;
  // uint32_t *tarlen = bamHdr->target_len ;

  // printf("%d\n",tar);

  int rc;

  std::cerr << "before while" << std::endl;
  while (!(rc = sam_read1(fp_in, bamHdr, aln))) {
    int32_t pos =
        aln->core.pos + 1;  // left most position of alignment in zero based coordinate (+1)
    char* chr = bamHdr->target_name[aln->core.tid];  // contig name (chromosome)
    uint32_t len = aln->core.l_qseq;                 // length of the read.

    uint8_t* q = bam_get_seq(aln);  // quality string
    uint32_t q2 = aln->core.qual;   // mapping quality

    char* qname = bam_get_qname(aln);
    uint16_t flag = aln->core.flag;
    uint32_t* cigar = bam_get_cigar(aln);
    uint32_t n_cigar = aln->core.n_cigar;
    char cigar_codes[] = {'M', 'I', 'D', 'N', 'S', 'H', 'P', '=', 'X'};
    // uint8_t* qual = bam_get_qual(aln);
    char* qual = (char*)bam_get_qual(aln);
    uint8_t mapq = aln->core.qual;
    // char* seq = (char*)bam_get_seq(aln);
    int32_t rnext = aln->core.mtid;
    int32_t pnext = aln->core.mpos;
    int32_t tlen = aln->core.isize;

    char* qseq = (char*)malloc(len);

    for (size_t i = 0; i < len; i++) {
      qseq[i] = seq_nt16_str[bam_seqi(q, i)];  // gets nucleotide id and converts them into IUPAC
                                               // id.
    }

    // printf("chr=%s\tpos=%d\tlen=%d\tqseq=%s\tq=%s\tq2=%d\n",chr,pos,len,qseq,q,q2);
    printf("qname=%s\tflag=%d\tchr=%s\tpos=%d\tmapq=%d\tlen=%d\tqseq=%s\tq2=%d\n", qname, flag, chr,
           pos, mapq, len, qseq, q2);
    std::cout << "cigar=";
    for (uint32_t i = 0; i < n_cigar; i++) {
      auto op_len = cigar[i] >> 4;
      auto op = cigar_codes[cigar[i] & 0b1111];

      std::cout << op_len << op << ", ";
    }
    std::cout << std::endl;
    std::cout << "rnext=" << rnext << "\tpnext=" << pnext << "\ttlen=" << tlen << std::endl;
    std::cerr << "after while rc is " << rc << std::endl;

    std::cout << "qual=" << std::endl;
    for (uint32_t i = 0; i < len; i++) {
      double q = qual[i];
      std::cout << pow(10, (q / -10)) << ", ";
    }
    std::cout << std::endl << std::endl;

    free(qseq);
  }
  bam_destroy1(aln);
  sam_close(fp_in);
}

GenomicMap::GenomicMap(const std::string& mapping_file) {
  if (FileUtility::is_file(mapping_file)) {
    initialize(std::make_shared<FileUtility>(mapping_file));
  }
}

GenomicMap::GenomicMap(std::shared_ptr<FileUtility> mapping_reader) { initialize(mapping_reader); }

void GenomicMap::initialize(std::shared_ptr<FileUtility> mapping_reader) {
  m_mapping_reader = mapping_reader;
  std::string str;
  int line_num = -1;
  while (m_mapping_reader->generalized_getline(str)) {
    line_num++;

    auto toks = split(str, "\t");
    if (toks.size() < 3) {
      std::cerr
          << "Warning: line " << line_num << " of mapping file " << m_mapping_reader->filename
          << " does not contain enough fields (at least 3), make sure the file is tab separated"
          << std::endl;
      std::cerr << "Note: mapping file may be subset of serialized schema" << std::endl;
      continue;
    }

    std::string contig_name;
    uint64_t length;
    uint64_t starting_index;

    try {
      contig_name = toks[0];
      length = std::stol(toks[1]);
      starting_index = std::stol(toks[2]);
    } catch (...) {
      std::cerr << "Warning: line " << line_num << " of mapping file " << m_mapping_reader->filename
                << " could not be parsed (2nd or 3rd field was not a valid uint64_t)" << std::endl;
      std::cerr << "Note: mapping file may be subset of serialized schema" << std::endl;
      continue;
    }

    contigs.emplace_back(contig_name, length, starting_index);
  }

  idxs_name.resize(contigs.size());
  std::iota(idxs_name.begin(), idxs_name.end(), 0);
  std::sort(idxs_name.begin(), idxs_name.end(),
            [&](auto l, auto r) { return contigs[l].name < contigs[r].name; });

  idxs_position.resize(contigs.size());
  std::iota(idxs_position.begin(), idxs_position.end(), 0);
  std::sort(idxs_position.begin(), idxs_position.end(),
            [&](auto l, auto r) { return contigs[l].starting_index < contigs[r].starting_index; });

  for (auto& c : contigs) {
    logger.debug("Contigs : name={} length={} starting_index={}", c.name, c.length,
                 c.starting_index);
  }
}

uint64_t GenomicMap::flatten(std::string contig_name, uint64_t offset) {
  auto it = std::lower_bound(idxs_name.begin(), idxs_name.end(), contig_name,
                             [&](auto l, auto r) { return contigs[l].name < r; });

  // int idx = std::distance(idxs_name.begin(), it);

  if (it != idxs_name.end() && contigs[*it].name == contig_name) {
    if (offset < contigs[*it].length) {
      return contigs[*it].starting_index + offset;
    } else {
      std::cerr << "Error, contig " << contig_name << " is only length " << contigs[*it].length
                << ", " << offset << "is out of bounds" << std::endl;
      exit(1);
    }
  } else {
    std::cerr << "Error, contig " << contig_name << " not found in mapping file "
              << m_mapping_reader->filename << std::endl;
    exit(1);
  }
}

bool equivalent_schema(const OmicsSchema& l, const OmicsSchema& r) {
  if (l.attributes.size() != r.attributes.size()) return false;

  for (auto li = l.attributes.begin(), ri = r.attributes.begin(); ri != r.attributes.end();
       li++, ri++) {
    if (li->first != ri->first) return false;
    if (li->second.type != ri->second.type) return false;
  }

  return true;
}

void GenomicMap::serialize(std::string path) {
  for (auto& c : contigs) {
    c.serialize(path);
  }
}

void OmicsSchema::serialize(std::string path) {
  FileUtility::write_file(path, std::string(OMICSDS_VERSION) + "\n", true);  // version

  auto write = [&](std::string str) { return FileUtility::write_file(path, str); };

  std::string order_str = (order == POSITION_MAJOR) ? "POSITION_MAJOR\n" : "SAMPLE_MAJOR\n";
  write(order_str);  // order
  std::string num_attributes_str = std::to_string(attributes.size()) + "\tattributes\n";
  write(num_attributes_str);
  // attributes
  for (auto& a : attributes) {
    write(a.first);  // attribute name
    write("\t");
    write(a.second.type_to_string());  // attribute type as string
    write("\t");
    write(a.second.length_to_string());  // attribute length as string
    write("\n");
  }

  genomic_map.serialize(path);
}

bool OmicsSchema::create_from_file(const std::string& filename) {
  std::shared_ptr<FileUtility> reader;
  try {
    reader = std::make_shared<FileUtility>(filename);
  } catch (const OmicsDSException& e) {
    logger.error("Cannot deserialize OmicsSchema {}", filename);
    return false;
  }

  std::string str;
  // version
  if (!reader->generalized_getline(str)) {
    std::cerr << "Error: cannot deserialize OmicsSchema, " << filename << " is empty" << std::endl;
    return false;
  }
  if (str != "v1") {
    std::cerr << "Note: while deserializing OmicsSchema encountered version " << str
              << ", only v1 is supported" << std::endl;
  }

  // order
  if (!reader->generalized_getline(str)) {
    std::cerr << "Error: cannot deserialize OmicsSchema from " << filename
              << ", issue reading order" << std::endl;
    return false;
  }
  if (str == "POSITION_MAJOR") {
    order = POSITION_MAJOR;
  } else if (str == "SAMPLE_MAJOR") {
    order = SAMPLE_MAJOR;
  } else {
    std::cerr << "Error: cannot deserialize OmicsSchema from " << filename
              << ", issue reading order" << std::endl;
    return false;
  }

  if (!reader->generalized_getline(str)) {
    std::cerr << "Error: cannot deserialize OmicsSchema from " << filename
              << ", issue reading number of attributes" << std::endl;
    return false;
  }
  int num_attributes = -1;
  try {
    num_attributes = std::stoi(split(str, "\t")[0]);
    if (num_attributes < 0) {
      throw std::runtime_error("number of attributes is negative");
    }
  } catch (...) {
    std::cerr << "Error: cannot deserialize OmicsSchema from " << filename
              << ", issue reading number of attributes" << std::endl;
    return false;
  }

  attributes.clear();
  for (int i = 0; i < num_attributes; i++) {
    if (!reader->generalized_getline(str)) {
      std::cerr << "Error: cannot deserialize OmicsSchema from " << filename
                << ", fewer attributes than reported (" << i << " of " << num_attributes << ")"
                << std::endl;
      return false;
    }
    auto tokens = split(str, "\t");
    if (tokens.size() < 3) {
      std::cerr << "Error: cannot deserialize OmicsSchema from " << filename
                << ", issue reading attribute " << i << std::endl;
      return false;
    }
    int length;
    try {
      length = std::stoi(tokens[2]);
    } catch (...) {
      length = -1;
    }

    attributes.emplace(tokens[0], OmicsFieldInfo(tokens[1], length));
  }

  genomic_map = GenomicMap(reader);
  return true;
}

// FIXME could potentially index map instead of doing this, only an issue if very many attributes
int OmicsSchema::index_of_attribute(const std::string& name) {
  auto it = attributes.find(name);
  if (it == attributes.end()) {
    return -1;
  }

  return std::distance(attributes.begin(), it);
}

OmicsCell OmicsCell::create_invalid_cell() {
  OmicsCell rv;
  rv.coords = {-1, -1};
  return rv;
}

bool OmicsCell::is_invalid_cell(const OmicsCell& cell) {
  return cell.coords[0] < 0 || cell.coords[1] < 0;
}

SamReader::SamReader(std::string filename, std::shared_ptr<OmicsSchema> schema,
                     std::shared_ptr<SampleMap> sample_map, int file_idx)
    : OmicsFileReader(filename, schema, sample_map, file_idx) {
  m_fp = hts_open(filename.c_str(), "r");  // open bam file
  m_hdr = sam_hdr_read(m_fp);              // read header
  m_align = bam_init1();                   // initialize an alignment

  if (!m_hdr) {
    std::cout << "SamReader header is null" << std::endl;
  }

  auto toks = split(filename, "/");
  std::string sample_name = "";
  if (toks.size()) {
    sample_name = toks.back();
  } else {
    std::cerr << "Error, could not deduce file name for file at path " << filename << std::endl;
  }
  if (m_sample_map->count(sample_name)) {
    m_row_idx = (*m_sample_map)[sample_name];
  } else {
    std::cerr << "Error, no entry " << sample_name << " in sample map" << std::endl;
    exit(1);
  }

  assert((bool)schema);
}

SamReader::~SamReader() {
  // std::cout << "REMOVE SamReader::~SamReader" << std::endl;
  // std::cout << "FIXME uncomment cleanup functions" << std::endl;
  bam_destroy1(m_align);
  sam_close(m_fp);
}

std::vector<OmicsCell> SamReader::get_next_cells() {
  std::vector<OmicsCell> cells;

  int rc;
  if (!(rc = sam_read1(m_fp, m_hdr, m_align))) {
    int32_t pos =
        m_align->core.pos + 1;  // left most position of alignment in zero based coordinate (+1)
    char* chr = m_hdr->target_name[m_align->core.tid];  // contig name (chromosome)
    uint32_t len = m_align->core.l_qseq;                // length of the read.

    uint8_t* q = bam_get_seq(m_align);  // quality string
    uint32_t q2 = m_align->core.qual;   // mapping quality

    char* qname = bam_get_qname(m_align);
    uint16_t flag = m_align->core.flag;
    uint32_t* cigar = bam_get_cigar(m_align);
    uint32_t n_cigar = m_align->core.n_cigar;
    char cigar_codes[] = {'M', 'I', 'D', 'N', 'S', 'H', 'P', '=', 'X'};
    // uint8_t* qual = bam_get_qual(m_align);

    char* qual = (char*)bam_get_qual(m_align);
    uint8_t mapq = m_align->core.qual;
    // char* seq = (char*)bam_get_seq(m_align);
    int32_t rnext = m_align->core.mtid;
    int32_t pnext = m_align->core.mpos;
    int32_t tlen = m_align->core.isize;
    std::vector<char> qseq(len);

    for (size_t i = 0; i < len; i++) {
      qseq[i] = seq_nt16_str[bam_seqi(q, i)];  // gets nucleotide id and converts them into IUPAC
                                               // id.
    }

    std::string sample = get_filename();

    int64_t position = m_schema->genomic_map.flatten(chr, pos);

    std::cerr << "\t\t\t\tREMOVE rname len " << std::strlen(chr) << std::endl;
    std::cerr << "\t\t\t\tREMOVE cigar len " << n_cigar << std::endl;

    OmicsCell cell({(int64_t)m_row_idx, position}, m_schema, m_file_idx);
    cell.add_field_ptr("QNAME", qname, std::strlen(qname));
    cell.add_field("FLAG", flag);
    cell.add_field_ptr("RNAME", chr, std::strlen(chr));
    cell.add_field("POS", pos);
    cell.add_field("MAPQ", mapq);
    cell.add_field_ptr("CIGAR", cigar, n_cigar);
    cell.add_field("RNEXT", rnext);
    cell.add_field("PNEXT", pnext);
    cell.add_field("TLEN", tlen);
    cell.add_field_ptr("SEQ", qseq.data(), (int)qseq.size());
    cell.add_field_ptr("QUAL", qual, std::strlen(qual));

    // FIXME REMOVE
    cell.add_field_ptr("SAMPLE_NAME", (char*)sample.c_str(), (int)sample.length());

    OmicsCell end_cell = cell;
    end_cell.file_idx = -1;
    int end_offset = std::abs(tlen) - 1;  // FIXME figure out negative template length
    end_cell.coords[1] += end_offset;
    if (tlen) {  // if end cell is in same position, only create one cell
      std::cout << "REMOVE SamReader::get_next_cells return " << cell.coords[1] << ", "
                << end_cell.coords[1] << std::endl;
      return {cell, end_cell};
    }
    std::cout << "REMOVE SamReader::get_next_cells return " << cell.coords[1] << std::endl;
    return {cell};
  } else {
    std::cout << "REMOVE sam_read1 was " << rc << std::endl;
    return {OmicsCell::create_invalid_cell()};
  }
}

BedReader::BedReader(std::string filename, std::shared_ptr<OmicsSchema> schema,
                     std::shared_ptr<SampleMap> sample_map, int file_idx)
    : OmicsFileReader(filename, schema, sample_map, file_idx) {
  std::string line;
  if (!m_reader_util->generalized_getline(line)) {
    std::cerr << "Error file " << filename << " is empty, skipping" << std::endl;
    return;
  }

  std::smatch m;
  std::regex exp("description\\s*=\\s*\"(.*?)\"");
  std::regex_search(line, m, exp);

  if (m.size() < 2) {
    std::cerr << "Error, could not find sample name for file " << filename << std::endl;
    std::cerr << "First line of Bed file should contain 'description = \"[sample_name]\"'"
              << std::endl;
    exit(1);
  }

  m_sample_name = m[1];
  std::cout << "************************************** BED sample is " << m_sample_name
            << std::endl;

  if (!m_sample_map->count(m_sample_name)) {
    std::cerr << "Error, sample " << m_sample_name << " from file " << filename
              << " not found in sample map" << std::endl;
    exit(1);
  }

  m_row_idx = (*m_sample_map)[m_sample_name];
}

std::vector<OmicsCell> BedReader::get_next_cells() {
  std::string line;
  while (m_reader_util->generalized_getline(line)) {
    std::stringstream ss(line);
    std::string str;
    std::vector<std::string> fields;
    while (ss >> str) {
      fields.push_back(str);
    }
    if (fields.size() < 5) {
      continue;
    }

    std::string chrom = fields[0], gene = "N/A", name = fields[3];
    uint64_t start, end, flattened_start, flattened_end;
    float score;

    try {
      start = std::stoul(fields[1]);
      flattened_start = m_schema->genomic_map.flatten(chrom, start);
      end = std::stoul(fields[2]);
      flattened_end = m_schema->genomic_map.flatten(chrom, end);
      score = std::stof(fields[4]);
    } catch (...) {
      continue;
    }

    OmicsCell cell({(int64_t)m_row_idx, (int64_t)flattened_start}, m_schema, m_file_idx);
    cell.add_field_ptr("CHROM", chrom.c_str(), chrom.length());
    cell.add_field("START", start);
    cell.add_field("END", end);
    cell.add_field("SCORE", score);
    cell.add_field_ptr("GENE", gene.c_str(), gene.length());
    cell.add_field_ptr("SAMPLE_NAME", m_sample_name.c_str(), m_sample_name.length());
    cell.add_field_ptr("NAME", name.c_str(), name.length());

    OmicsCell end_cell = cell;
    end_cell.file_idx = -1;
    end_cell.coords[1] = flattened_end;

    if (flattened_start == flattened_end) {
      return {cell};
    }
    return {cell, end_cell};
  }
  return {};
}

MatrixReader::MatrixReader(std::string filename, std::shared_ptr<OmicsSchema> schema,
                           std::shared_ptr<SampleMap> sample_map, int file_idx)
    : OmicsFileReader(filename, schema, sample_map, file_idx) {
  std::string line;

  if (!m_reader_util->generalized_getline(line)) {
    logger.fatal(OmicsDSException(logger.format("Matrix file {} is empty", filename)));
  }

  auto toks = split(line, m_token_separator);
  if (toks[0] == "GENE") {
    m_id_major = false;
  } else if (toks[0] == "SAMPLE") {
    m_id_major = true;
  } else {
    logger.fatal(OmicsDSException(
        logger.format("Error reading matrix file {}. {}", filename,
                      "\nMatrix files should have either \n\t \"GENE\t[gene1]\t[gene2]\"\n  OR\n\t "
                      "\"SAMPLE\t[sample1]\t[sample2]\"\n  for a header")));
  }

  if (m_id_major != m_schema->position_major()) {
    logger.fatal(OmicsDSException(
        logger.format("Error order of matrix file {} does not match that of schema, they should "
                      "both be either gene/transcript id major or sample major",
                      filename)));
  }

  m_columns = std::vector<std::string>(toks.begin() + 1, toks.end());
  m_row_scores = std::vector<float>(m_columns.size(), 0);
  m_column_idx = m_columns.size();  // to force parsing next line
}

bool MatrixReader::parse_next(std::string& sample, std::string& gene, float& score) {
  if (m_column_idx >= m_row_scores.size()) {
    std::string line;
    if (!m_reader_util->generalized_getline(line)) {
      logger.debug("*** End of input!!!");
      return false;
    }

    auto toks = split(line, m_token_separator);
    if (toks.size() != m_columns.size() + 1) {  // sample/gene id followed by scores
      logger.fatal(OmicsDSException(logger.format("Error with matrix cell values in input, number "
                                                  "of columns({}) do not match header({})",
                                                  toks.size() - 1, m_columns.size())),
                   "Erroneous line from file({}) : \n{}{}", get_filename(), line.substr(0, 60),
                   line.length() > 59 ? "..." : "");
    }

    m_current_token = toks[0];

    try {
      for (size_t i = 0; i < m_columns.size(); i++) {
        m_row_scores[i] = std::stof(toks[i + 1]);
      }
    } catch (...) {
      logger.fatal(
          OmicsDSException("Error with matrix cell values in input, they have to be numbers"));
    }
    m_column_idx = 0;
  }

  if (m_column_idx == m_row_scores.size()) {
    return false;
  }

  if (m_id_major) {
    sample = m_columns[m_column_idx];
    gene = m_current_token;
  } else {
    sample = m_current_token;
    gene = m_columns[m_column_idx];
  }
  score = m_row_scores[m_column_idx];
  m_column_idx++;
  return true;
}

std::vector<OmicsCell> MatrixReader::get_next_cells() {
  std::string sample_name, gene_name;  // TODO avoid repeated lookups for row token
  float score;
  uint64_t row_idx;

  while (parse_next(sample_name, gene_name, score)) {
    if (m_sample_map->count(sample_name)) {
      gtf_encoding_t encoded_id = encode_gtf_id(gene_name);
      logger.debug("Gene={} Encoded ID={:#08x} {:#08x}", gene_name, encoded_id.first,
                   encoded_id.second);
      if (encoded_id.first) {
        row_idx = (*m_sample_map)[sample_name];
        MatrixCell cell({(int64_t)row_idx, (int64_t)encoded_id.first}, encoded_id.second, m_schema,
                        m_file_idx);
        cell.add_field("SCORE", score);
        cell.add_field("VERSION", encoded_id.second);
        return {cell};
      } else {
        logger.error("Gene name {} cannot be encoded", gene_name);
      }
    }
  }
  return {};
}

void OmicsLoader::store_buffers() {
  std::vector<void*> buffers_vec;
  std::vector<size_t> buffer_sizes_vec;

  int i = 0;
  for (auto it = m_schema->attributes.begin(); it != m_schema->attributes.end(); it++, i++) {
    buffers_vec.push_back(m_buffers[i].data());
    buffer_sizes_vec.push_back(m_buffer_lengths[i]);

    if (it->second.is_variable()) {
      buffers_vec.push_back(m_var_buffers[i].data());
      buffer_sizes_vec.push_back(m_var_buffer_lengths[i]);
    }
  }

  buffers_vec.push_back(m_coords_buffer.data());
  buffer_sizes_vec.push_back(m_coords_buffer_length * sizeof(size_t));

  m_array_storage->store(buffers_vec, buffer_sizes_vec);
}

static std::string format_number(uint64_t number) {
  std::vector<std::string> suffix = {"", "K", "M", "G", "T"};
  auto i = 0u;
  while (number > 0) {
    if (i > suffix.size()) {
      i--;
      break;
    }
    if (number / 1024 == 0) {
      return std::to_string(number) + suffix[i];
    } else {
      number /= 1024;
      i++;
    }
  }
  return std::to_string(number) + suffix[i];
}

void OmicsLoader::write_buffers() {
  store_buffers();

  m_total_processed_cells += m_buffered_cells;
  logger.info("Processed {}/{} cells", format_number(m_total_processed_cells),
              format_number(252000000));
  // Reset buffers' lengths
  for (auto i = 0u; i < m_schema->attributes.size(); i++) {
    m_buffer_lengths[i] = 0;
    m_var_buffer_lengths[i] = 0;
  }
  m_coords_buffer_length = 0;
  m_buffered_cells = 0;
  memset(m_attribute_offsets.data(), 0, m_attribute_offsets.size() * sizeof(size_t));
}

bool OmicsLoader::check_buffer_sizes(const OmicsCell& cell) {
  assert(cell.fields.size() == m_schema->attributes.size());
  for (auto [i, attribute] = std::tuple{0u, m_schema->attributes.begin()};
       i < m_schema->attributes.size(); i++, attribute++) {
    auto& field = cell.fields[i].data;
    size_t size = attribute->second.element_size();
    if (attribute->second.is_variable()) {  // variable length
      if (m_buffer_lengths[i] + sizeof(size_t) > buffer_size) return false;
      if (m_var_buffer_lengths[i] + field.size() > buffer_size) return false;
    } else {
      if (m_buffer_lengths[i] + field.size() > buffer_size) return false;
    }
  }
  if (m_coords_buffer_length + 3 * sizeof(uint64_t) > buffer_size) return false;
  return true;
}

void OmicsLoader::buffer_cell(const OmicsCell& cell, int level) {
  if (!check_buffer_sizes(cell)) {
    write_buffers();
  }

  for (auto [i, attribute] = std::tuple{0u, m_schema->attributes.begin()};
       i < m_schema->attributes.size(); i++, attribute++) {
    auto& field = cell.fields[i].data;
    int length = attribute->second.length;
    size_t size = attribute->second.element_size();
    if (attribute->second.is_variable()) {  // variable length
      memcpy(m_buffers[i].data() + m_buffer_lengths[i], &m_attribute_offsets[i], sizeof(size_t));
      memcpy(m_var_buffers[i].data() + m_var_buffer_lengths[i], field.data(), field.size());
      m_attribute_offsets[i] += field.size();
      m_buffer_lengths[i] += sizeof(size_t);
      m_var_buffer_lengths[i] += field.size();
    } else {
      assert(length >= 0);  // Should only be negative if variable
      assert(field.size() == size * (size_t)length);
      memcpy(m_buffers[i].data() + m_buffer_lengths[i], field.data(), field.size());
      m_buffer_lengths[i] += field.size();
    }
  }

  memcpy(m_coords_buffer.data() + m_coords_buffer_length++, &cell.coords[0], sizeof(uint64_t));
  memcpy(m_coords_buffer.data() + m_coords_buffer_length++, &cell.coords[1], sizeof(uint64_t));
  uint64_t this_level = (uint64_t)level;
  memcpy(m_coords_buffer.data() + m_coords_buffer_length++, &this_level, sizeof(uint64_t));

  bool close_and_reopen_array = less_than(m_pq.top(), cell);
  if (++m_buffered_cells > 1024 * 1024 * 1 /*1M cells ~320MB for single cell*/ ||
      close_and_reopen_array) {
    if (close_and_reopen_array && !m_split_warning_emitted && !m_pq.empty()) {
      m_split_warning_emitted = true;
      logger.warn(
          "Array is being split over multiple fragments. This may cause perfomance penalties "
          "while querying. Consider consolidating the array after import.");
    }
    write_buffers();
  }
  if (close_and_reopen_array) reopen_array();
}

void ReadCountLoader::create_schema() {
  m_schema->attributes.emplace("SAMPLE_NAME",
                               OmicsFieldInfo(OmicsFieldInfo::OmicsFieldType::omics_char, -1));
  m_schema->attributes.emplace("QNAME",
                               OmicsFieldInfo(OmicsFieldInfo::OmicsFieldType::omics_char, -1));
  m_schema->attributes.emplace("FLAG",
                               OmicsFieldInfo(OmicsFieldInfo::OmicsFieldType::omics_uint16_t, 1));
  m_schema->attributes.emplace("RNAME",
                               OmicsFieldInfo(OmicsFieldInfo::OmicsFieldType::omics_char, -1));
  m_schema->attributes.emplace("POS",
                               OmicsFieldInfo(OmicsFieldInfo::OmicsFieldType::omics_int32_t, 1));
  m_schema->attributes.emplace("MAPQ",
                               OmicsFieldInfo(OmicsFieldInfo::OmicsFieldType::omics_uint8_t, 1));
  m_schema->attributes.emplace("CIGAR",
                               OmicsFieldInfo(OmicsFieldInfo::OmicsFieldType::omics_uint32_t, -1));
  m_schema->attributes.emplace("RNEXT",
                               OmicsFieldInfo(OmicsFieldInfo::OmicsFieldType::omics_int32_t, 1));
  m_schema->attributes.emplace("PNEXT",
                               OmicsFieldInfo(OmicsFieldInfo::OmicsFieldType::omics_int32_t, 1));
  m_schema->attributes.emplace("TLEN",
                               OmicsFieldInfo(OmicsFieldInfo::OmicsFieldType::omics_int32_t, 1));
  m_schema->attributes.emplace("SEQ",
                               OmicsFieldInfo(OmicsFieldInfo::OmicsFieldType::omics_char, -1));
  m_schema->attributes.emplace("QUAL",
                               OmicsFieldInfo(OmicsFieldInfo::OmicsFieldType::omics_char, -1));
}

void ReadCountLoader::add_reader(const std::string& filename) {
  if (std::regex_match(filename, std::regex("(.*)(sam)($)"))) {
    m_files.push_back(
        std::make_shared<SamReader>(filename, m_schema, m_sample_map, m_files.size()));
  }
}

void TranscriptomicsLoader::create_schema() {
  m_schema->attributes.emplace("CHROM",
                               OmicsFieldInfo(OmicsFieldInfo::OmicsFieldType::omics_char, -1));
  m_schema->attributes.emplace("START",
                               OmicsFieldInfo(OmicsFieldInfo::OmicsFieldType::omics_uint64_t, 1));
  m_schema->attributes.emplace("END",
                               OmicsFieldInfo(OmicsFieldInfo::OmicsFieldType::omics_uint64_t, 1));
  m_schema->attributes.emplace("SCORE",
                               OmicsFieldInfo(OmicsFieldInfo::OmicsFieldType::omics_float_t, 1));
  //  m_schema->attributes.emplace("GENE",
  //  OmicsFieldInfo(OmicsFieldInfo::OmicsFieldType::omics_char, -1)); // Will be N/A for bed
  //  files
  m_schema->attributes.emplace("SAMPLE_NAME",
                               OmicsFieldInfo(OmicsFieldInfo::OmicsFieldType::omics_char, -1));
  m_schema->attributes.emplace(
      "NAME", OmicsFieldInfo(OmicsFieldInfo::OmicsFieldType::omics_char,
                             -1));  // Field in bed files, will be N/A for matrix files
}

void TranscriptomicsLoader::add_reader(const std::string& filename) {
  if (std::regex_match(filename, std::regex("(.*)(bed)($)"))) {
    m_files.push_back(
        std::make_shared<BedReader>(filename, m_schema, m_sample_map, m_files.size()));
  }
  //  else if(std::regex_match(filename, std::regex("(.*)(resort)($)"))) {
  // m_files.push_back(std::make_shared<MatrixReader>(filename, m_schema, m_sample_map,
  // m_gene_id_map, m_files.size()));
  // }
}

void MatrixLoader::create_schema() {
  m_schema->attributes.emplace("SCORE",
                               OmicsFieldInfo(OmicsFieldInfo::OmicsFieldType::omics_float_t, 1));
}

void MatrixLoader::add_reader(const std::string& filename) {
  m_files.push_back(
      std::make_shared<MatrixReader>(filename, m_schema, m_sample_map, m_files.size()));
}

GeneIdMap::GeneIdMap(const std::string& gene_map, std::shared_ptr<OmicsSchema> schema,
                     bool use_transcript, bool drop_version)
    : schema(schema) {
  auto toks = split(gene_map, ".");
  if (!toks.size()) {
    std::cerr << "Error creating gene map, cannot deduce file format from filename " << gene_map
              << std::endl;
    exit(1);
  }

  std::string format = toks.back();

  if (format == "gtf" || format == "gi") {
    create_from_gtf(gene_map, use_transcript, drop_version);
    return;
  }
  if (format == "gi") {
    create_from_gi(gene_map);
    return;
  }
  if (format == "gbed") {
    create_from_gbed(gene_map);
    return;
  }
  std::cerr << "Error creating gene map, unsupported file format for filename " << gene_map
            << " (expected gtf/gff or gi)" << std::endl;
  exit(1);
}

void GeneIdMap::create_from_gtf(const std::string& gene_map, bool use_transcript,
                                bool drop_version) {
  std::string str;

  int ind = -1;

  FileUtility file(gene_map);

  while (file.generalized_getline(str)) {
    if (str[0] == '#') {
      continue;
    }

    ++ind;

    std::stringstream ss(str);
    std::string field;
    std::vector<std::string> fields;

    // read the 8 fields before attribute
    for (int i = 0; i < 8; i++) {
      ss >> field;
      fields.push_back(field);
    }

    if (fields[2] != "transcript") {
      continue;
    }

    std::string chrom = fields[0];
    uint64_t start, end, flattened_start, flattened_end;

    try {
      start = std::stoul(fields[3]);
      end = std::stoul(fields[4]);
      flattened_start = schema->genomic_map.flatten(chrom, start);
      flattened_end = schema->genomic_map.flatten(chrom, end);
    } catch (...) {
      continue;
    }

    std::string attributes;
    std::getline(ss, attributes);

    std::string pattern = use_transcript
                              ? "transcript_id"
                              : "gene_id";  // FIXME use regex in case strangeness with whitespace

    std::smatch m;
    std::regex exp(pattern + "\\s*\"(.*?)\"");
    std::regex_search(str, m, exp);

    if (m.size() < 2) {
      continue;
    }

    std::string tid = m[1];

    if (drop_version) {
      tid = split(tid, ".")[0];
    }

    if (map.count(tid)) {
      std::cout << "gtf reading: " << tid
                << " is duplicate, old start/end: " << map[tid].flattened_start << ", "
                << map[tid].flattened_end << ", new start/end: " << flattened_start << ", "
                << flattened_end << std::endl;
    }

    map[tid] = Gene(chrom, start, end, flattened_start, flattened_end);
  }
}

void GeneIdMap::create_from_gi(const std::string& gene_map) {
  FileUtility file(gene_map);

  char version;
  file.read_file(&version, 1);
  if (version != 1) {
    std::cout << "version " << (int)version << " not supported" << std::endl;
  }

  int64_t size = 0;
  file.read_file(((char*)&size), 5);

  for (int i = 0; i < size; i++) {
    uint16_t len;  // gene name
    file.read_file((char*)&len, 2);
    char name[len];
    file.read_file(name, len);
    uint16_t len2;  // chrom
    file.read_file((char*)&len2, 2);
    char chrom[len2];
    file.read_file(chrom, len2);
    uint64_t start = 0;
    uint64_t end = 0;
    file.read_file((char*)&start, 5);
    file.read_file((char*)&end, 5);
    map[std::string(name, len)] =
        Gene(std::string(chrom, len2), start, end, schema->genomic_map.flatten(chrom, start),
             schema->genomic_map.flatten(chrom, end));
  }
}

void GeneIdMap::create_from_gbed(const std::string& gene_map) {
  FileUtility file(gene_map);

  std::string str;
  while (file.generalized_getline(str)) {
    auto toks = split(str, "\t");
    if (toks.size() < 4) {
      continue;
    }
    std::string chrom = toks[1];
    uint64_t start, end;
    try {
      start = std::stoul(toks[2]);
      end = std::stoul(toks[3]);
    } catch (...) {
      continue;
    }
    map[toks[0]] = Gene(chrom, start, end, schema->genomic_map.flatten(chrom, start),
                        schema->genomic_map.flatten(chrom, end));
  }
}

void GeneIdMap::export_as_gi(const std::string& filename) {
  auto write = [&](const void* buffer, size_t length) {
    return FileUtility::write_file(filename, buffer, length);
  };

  char version = 1;
  write(&version, 1);

  // write 5B number of entries
  int64_t size = map.size();
  write((char*)&size, 5);

  // write 2B name length, name, 2B chrom length, chrom, 5B start, 5B end
  for (auto& a : map) {
    std::string name = a.first;
    int16_t len = name.length();
    uint64_t start = a.second.start;
    uint64_t end = a.second.end;
    std::string chrom = a.second.chrom;
    int16_t len2 = chrom.length();

    write((char*)&len, 2);
    write(name.c_str(), len);
    write((char*)&len2, 2);
    write(chrom.c_str(), len2);
    write((char*)&start, 5);
    write((char*)&end, 5);
  }
}

// FIXME add parallelism of some kind
// FIXME
OmicsLoader::OmicsLoader(const std::string& workspace, const std::string& array,
                         const std::string& file_list, const std::string& sample_map,
                         const std::string& mapping_file, bool position_major)
    : OmicsDSModule(workspace, array, mapping_file, position_major),
      m_file_list(file_list),
      m_sample_map(std::make_shared<SampleMap>(sample_map)),
      m_pq(comparator) {}

void OmicsLoader::initialize() {  // FIXME move file reader creation to somewhere virtual
  create_schema();

  m_array_storage->initialize(true, m_schema, true, true);

  // set up buffers
  m_buffers = std::vector<std::vector<uint8_t>>(m_schema->attributes.size());
  m_var_buffers = std::vector<std::vector<uint8_t>>(m_schema->attributes.size());
  m_attribute_offsets = std::vector<size_t>(m_schema->attributes.size(), 0);

  for (auto i = 0u; i < m_schema->attributes.size(); i++) {
    m_buffers[i].resize(buffer_size);
    m_var_buffers[i].resize(buffer_size);
  }
  m_coords_buffer.resize(3 * buffer_size);
  m_buffer_lengths.resize(m_schema->attributes.size(), 0);
  m_var_buffer_lengths.resize(m_schema->attributes.size(), 0);
  m_coords_buffer_length = 0;

  serialize_schema();

  // add file readers
  FileUtility list(m_file_list);
  std::string filename;
  while (list.generalized_getline(filename)) {
    if (FileUtility::is_file(filename)) {
      add_reader(filename);
    }
  }

  // push first cells from all files
  push_from_all_files();
}

void OmicsLoader::push_from_all_files() {
  for (auto& f : m_files) {
    if (!f) continue;

    auto cells = f->get_next_cells();
    for (auto& c : cells) {
      if (OmicsCell::is_invalid_cell(c)) continue;
      c.coords = m_schema->swap_order(c.coords);  // to schema order
      logger.debug("1 Cell coords {:#08x} {:#08x} from file {}", c.coords[0], c.coords[1],
                   f->get_filename());
      m_pq.push(c);
    }
  }
}

void OmicsLoader::push_from_idxs(const std::set<int>& idxs) {
  for (auto& idx : idxs) {
    if (idx < 0) continue;

    auto cells = m_files[idx]->get_next_cells();
    for (auto& c : cells) {
      if (OmicsCell::is_invalid_cell(c)) continue;
      c.coords = m_schema->swap_order(c.coords);  // to schema order
      logger.debug("2 Cell coords {:#08x} {:#08x} from file {}", c.coords[0], c.coords[1], idx);
      m_pq.push(c);
    }
  }
}

void OmicsLoader::push_file_from_cell(const OmicsCell& cell) { push_from_idxs({cell.file_idx}); }

void OmicsLoader::import() {
  std::cout << "OmicsLoader::import" << std::endl;

  std::cout << "REMOVE beginning m_pq.size() is " << m_pq.size() << std::endl;

  std::array<int64_t, 2> last_coords = {-1, -1};
  int level = 0;

  while (m_pq.size()) {
    auto cell = m_pq.top();
    std::cout << "\t\t\tREMOVE top coords are " << container_to_string(m_pq.top().coords)
              << std::endl;
    m_pq.pop();
    push_file_from_cell(cell);  // make sure that all files are still represented in pq

    if (cell.coords[0] < 0 || cell.coords[1] < 0) {  // invalid cell
      continue;
    }

    if (less_than(m_pq.top(), cell)) {
      std::cerr << "Error, top of priority queue is less than previous cell" << std::endl;
      std::cerr << "prev: " << container_to_string(cell.coords) << std::endl;
      std::cerr << "top: " << container_to_string(m_pq.top().coords) << std::endl;
      exit(1);
    }

    if (cell.coords == last_coords) {
      level++;
    } else {
      level = 0;
      last_coords = cell.coords;
    }

    // Buffer cell. Persist to storage if number of cells has passed threshold and clear buffers.
    buffer_cell(cell, level);
  }
  // Persist remaining cells in buffers.
  store_buffers();
}

void MatrixLoader::import() {
  logger.info("Starting import...");
  while (m_pq.size()) {
    auto cell = m_pq.top();
    m_pq.pop();
    push_file_from_cell(cell);
    MatrixCell* matrix_cell = (MatrixCell*)&cell;
    expand_extent(Dimension::SAMPLE, matrix_cell->coords[1]);
    expand_extent(Dimension::FEATURE, matrix_cell->coords[0]);
    buffer_cell(cell, matrix_cell->get_version());
  }
  write_buffers();
  logger.info("Import DONE");
}

extents_t MatrixLoader::get_extent(Dimension dimension) {
  return m_array_metadata->get_extent(dimension);
}

extents_t MatrixLoader::expand_extent(Dimension dimension, size_t value) {
  return m_array_metadata->expand_extent(dimension, value);
}

std::shared_ptr<ArrayMetadata> MatrixLoader::default_metadata() {
  std::shared_ptr<ArrayMetadata> metadata = std::make_shared<ArrayMetadata>();
  Dimension dim = Dimension::SAMPLE;
  generate_default_extent(metadata->add_extents(), &dim);
  dim = Dimension::FEATURE;
  generate_default_extent(metadata->add_extents(), &dim);
  return metadata;
}

void MatrixLoader::generate_default_extent(DimensionExtent* dimension_extent,
                                           Dimension* dimension) {
  dimension_extent->set_dimension(*dimension);
  Extent* extent = dimension_extent->mutable_extent();
  extent->set_start(-1ul);
  extent->set_end(0ul);
}

std::shared_ptr<OmicsLoader> get_loader(std::string_view workspace, std::string_view array,
                                        OmicsDSImportConfig config) {
  std::shared_ptr<OmicsLoader> loader = NULL;
  OmicsDSConfigure workspace_config(workspace);
  OmicsDSImportConfig import_config = workspace_config.get_import_config();
  import_config.merge(config);

  if (!import_config.file_list || !import_config.sample_map || !import_config.import_type) {
    return loader;
  }

  switch (*import_config.import_type) {
    case OmicsDSImportType::FEATURE_IMPORT:
      loader = std::make_shared<MatrixLoader>(workspace.data(), array.data(),
                                              *import_config.file_list, *import_config.sample_map);
      break;

    case OmicsDSImportType::READ_IMPORT:
      if (!import_config.mapping_file) {
        return loader;
      }
      loader = std::make_shared<ReadCountLoader>(
          workspace.data(), array.data(), *import_config.file_list, *import_config.sample_map,
          *import_config.mapping_file, !import_config.sample_major);
      break;
    case OmicsDSImportType::INTERVAL_IMPORT:
      if (!import_config.mapping_file) {
        return loader;
      }
      loader = std::make_shared<TranscriptomicsLoader>(
          workspace.data(), array.data(), *import_config.file_list, *import_config.sample_map,
          *import_config.mapping_file, "", !import_config.sample_major);
      break;
  }
  return loader;
}
