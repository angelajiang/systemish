#include <hs/hs.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <boost/algorithm/string.hpp>
#include <fstream>
#include <string>

#include "dfc_src/dfc.h"
#include "dfc_src/dfc_framework.h"
#include "utils/timer.h"

static constexpr size_t kNumPkts = 30000;
static constexpr size_t kPktSize = 1024;

double freq_ghz = 0;

struct byte_arr_t {
  size_t num_bytes;
  unsigned char *byte_arr;

  byte_arr_t(size_t num_bytes, unsigned char *byte_arr)
      : num_bytes(num_bytes), byte_arr(byte_arr) {}

  // Generate from a string of space-separated bytes
  static byte_arr_t gen_from_string(std::string &byte_string) {
    std::vector<std::string> byte_vec;
    boost::split(byte_vec, byte_string, boost::is_any_of(" "));

    size_t num_bytes = byte_vec.size();
    rt_assert(num_bytes > 0);
    auto *byte_arr = new unsigned char[num_bytes];
    for (size_t i = 0; i < num_bytes; i++) {
      byte_arr[i] = std::stoi(byte_vec.at(i));
    }

    return byte_arr_t(num_bytes, byte_arr);
  }

  void print() {
    printf("%zu |", num_bytes);
    for (size_t i = 0; i < num_bytes; i++) {
      printf("%u ", byte_arr[i]);
    }
    printf("\n");
  }
};

std::vector<byte_arr_t *> virus_vec;
std::vector<byte_arr_t *> pkt_vec;

static size_t match_count = 0;
static int hs_ev_handler(unsigned int, unsigned long long, unsigned long long,
                         unsigned int, void *) {
  match_count++;
  return 0;
}
/*
void evaluate_hyperscan() {
  hs_database_t *db;
  hs_compile_error_t *compile_err;

  unsigned int *ids = new unsigned int[virus_vec.size()];
  char **virus_arr = new char *[virus_vec.size()];
  for (size_t i = 0; i < virus_vec.size(); i++) {
    ids[i] = i;
    virus_arr[i] = (char *)virus_vec[i].c_str();
  }

  int ret = hs_compile_multi(virus_arr, nullptr, ids, virus_vec.size(),
                             HS_MODE_BLOCK, nullptr, &db, &compile_err);
  if (ret != HS_SUCCESS) {
    fprintf(stderr, "ERROR: Unable to compile\n");
    hs_free_compile_error(compile_err);
    exit(-1);
  }

  hs_scratch_t *scratch = nullptr;
  if (hs_alloc_scratch(db, &scratch) != HS_SUCCESS) {
    fprintf(stderr, "ERROR: Unable to allocate scratch space. Exiting.\n");
    hs_free_database(db);
    exit(-1);
  }

  TscTimer timer;
  timer.start();

  size_t num_pkts_matched = 0;
  for (size_t i = 0; i < kNumPkts; i++) {
    match_count = 0;
    if (hs_scan(db, pkt_vec[i], kPktSize, 0, scratch, hs_ev_handler, nullptr) !=
        HS_SUCCESS) {
      fprintf(stderr, "ERROR: Unable to scan input buffer. Exiting.\n");
      hs_free_scratch(scratch);
      exit(-1);
    }

    // printf("HS: Packet %zu: match = %u\n", i, (match_count > 0));
    num_pkts_matched += (match_count > 0);
  }

  timer.stop();

  printf("HyperScan: packets matched: %zu, bandwidth = %.3f GB/s\n",
         num_pkts_matched,
         (kPktSize * kNumPkts) / (1000000000.0 * timer.avg_sec(freq_ghz)));

  hs_free_scratch(scratch);
  hs_free_database(db);
}

void dfc_ev_handler(unsigned char *, uint32_t *, uint32_t list_size) {
  match_count = list_size;
}

void evaluate_dfc() {
  DFC_STRUCTURE *dfc = DFC_New();

  size_t pattern_id = 0;
  for (std::string &virus : virus_vec) {
    DFC_AddPattern(dfc, (unsigned char *)virus.c_str(), strlen(virus.c_str()),
                   0, pattern_id++);
  }

  DFC_Compile(dfc);

  TscTimer timer;
  timer.start();

  size_t num_pkts_matched = 0;
  for (size_t i = 0; i < kNumPkts; i++) {
    match_count = 0;
    DFC_Search(dfc, (unsigned char *)pkt_vec[i], kPktSize, dfc_ev_handler);

    // printf("DFC: Packet %zu: match = %u\n", i, (match_count > 0));
    num_pkts_matched += (match_count > 0);
  }

  timer.stop();

  printf("DFC: number of matches: %zu, bandwidth = %.3f GB/s\n",
         num_pkts_matched,
         (kPktSize * kNumPkts) / (1000000000.0 * timer.avg_sec(freq_ghz)));
}
*/

int main() {
  freq_ghz = measure_rdtsc_freq();
  printf("Kicking up TurboBoost. freq_ghz = %.2f\n", freq_ghz);
  nano_sleep(2000000000, freq_ghz);
  printf("Starting work!\n");

  // Get the list of viruses
  std::ifstream virus_file("test_patterns/test_bytes.txt");
  while (true) {
    std::string virus;
    std::getline(virus_file, virus);
    if (virus.empty()) break;
    virus_vec.emplace_back(byte_arr_t::gen_from_string(virus));
  }

  printf("Number of virus = %zu:\n", virus_vec.size());
  for (auto &byte_arr : virus_vec) byte_arr->print();

  // Generate random packets
  /*pkt_vec.resize(kNumPkts);
  for (size_t i = 0; i < kNumPkts; i++) {
    pkt_vec[i] = new char[kPktSize];
    for (size_t j = 0; j < kPktSize; j++) {
      pkt_vec[i][j] = static_cast<char>(rand() % 128);
    }
  }*/

  std::ifstream pkt_file("packets.txt");
  while (true) {
    std::string pkt;
    std::getline(pkt_file, pkt);
    if (pkt.empty()) break;
    pkt_vec.emplace_back(byte_arr_t::gen_from_string(pkt));
  }

  printf("Number of packets = %zu:\n", pkt_vec.size());
  for (auto &byte_arr : pkt_vec) byte_arr->print();

  // evaluate_hyperscan();
  // evaluate_dfc();
  return 0;
}
