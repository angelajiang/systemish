#include <hs/hs.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <string>

#include "utils/timer.h"

static constexpr size_t kStringLength = 1000000;
double freq_ghz = 0;
std::vector<std::string> virus_vec;

static int match_count = 0;
static int event_handler(unsigned int, unsigned long long, unsigned long long,
                         unsigned int, void *) {
  match_count++;
  return 0;
}

void evaluate_hyperscan(const char *string, const char *regex) {
  match_count = 0;

  hs_database_t *database;
  hs_compile_error_t *compile_err;

  std::ifstream in("virus.txt");
  while (true) {
    std::string s;
    std::getline(in, s);
    if (s.empty()) break;
    virus_vec.push_back(s);
    printf("Virus: %s\n", s.c_str());
  }

  for (std::string &virus : virus_vec) {
    if (hs_compile(virus.c_str(), 0, HS_MODE_BLOCK, nullptr, &database,
                   &compile_err) != HS_SUCCESS) {
      fprintf(stderr, "ERROR: Unable to compile regex \"%s\": %s\n", regex,
              compile_err->message);
      hs_free_compile_error(compile_err);
      exit(-1);
    }
  }

  hs_scratch_t *scratch = nullptr;
  if (hs_alloc_scratch(database, &scratch) != HS_SUCCESS) {
    fprintf(stderr, "ERROR: Unable to allocate scratch space. Exiting.\n");
    hs_free_database(database);
    exit(-1);
  }

  TscTimer timer;
  timer.start();

  unsigned int string_len = strlen(string);
  if (hs_scan(database, string, string_len, 0, scratch, event_handler,
              nullptr) != HS_SUCCESS) {
    fprintf(stderr, "ERROR: Unable to scan input buffer. Exiting.\n");
    hs_free_scratch(scratch);
    exit(-1);
  }

  timer.stop();

  printf("HyperScan: number of matches: %d, bandwidth = %.3f GB/s\n",
         match_count, kStringLength / (1000000000.0 * timer.avg_sec(freq_ghz)));

  hs_free_scratch(scratch);
  hs_free_database(database);
}

int main() {
  freq_ghz = measure_rdtsc_freq();
  printf("Kicking up TurboBoost. freq_ghz = %.2f\n", freq_ghz);
  nano_sleep(2000000000, freq_ghz);
  printf("Starting work!\n");

  auto *string = new char[kStringLength];
  for (size_t i = 0; i < kStringLength; i++) {
    string[i] = '0' + (rand() % 10);
  }

  const char *regex = "asdada";
  evaluate_hyperscan(string, regex);

  return 0;
}
