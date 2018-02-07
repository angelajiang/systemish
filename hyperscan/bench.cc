#include <hs/hs.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "timer.h"

static constexpr size_t kStringLength = 10000000;
double freq_ghz = 0;

static int count = 0;
static int eventHandler(unsigned int, unsigned long long, unsigned long long,
                        unsigned int, void *) {
  count++;
  return 0;
}

int hs_find_all(const char *pattern, const char *subject) {
  unsigned int subject_len = strlen(subject);
  hs_database_t *database;
  hs_compile_error_t *compile_err;
  if (hs_compile(pattern, 0, HS_MODE_BLOCK, nullptr, &database, &compile_err) !=
      HS_SUCCESS) {
    fprintf(stderr, "ERROR: Unable to compile pattern \"%s\": %s\n", pattern,
            compile_err->message);
    hs_free_compile_error(compile_err);
    return -1;
  }

  hs_scratch_t *scratch = nullptr;
  if (hs_alloc_scratch(database, &scratch) != HS_SUCCESS) {
    fprintf(stderr, "ERROR: Unable to allocate scratch space. Exiting.\n");
    hs_free_database(database);
    return -1;
  }

  if (hs_scan(database, subject, subject_len, 0, scratch, eventHandler,
              nullptr) != HS_SUCCESS) {
    fprintf(stderr, "ERROR: Unable to scan input buffer. Exiting.\n");
    hs_free_scratch(scratch);
    return 0;
  }

  hs_free_scratch(scratch);
  hs_free_database(database);
  return 1;
}

void evaluate_hyperscan(const char *string) {
  for (size_t n = 1; n < 32; n++) {
    // n is the number of (0|1) at the end of the regex
    std::string regex;
    regex += "(0|1)*1";
    for (size_t i = 0; i < n; i++) regex += "(0|1)";

    if (kStringLength < 50) {
      printf("string = %s, regex = %s\n", string, regex.c_str());
    }

    TscTimer timer;
    timer.start();
    hs_find_all(regex.c_str(), string);
    timer.stop();

    printf("HyperScan: n = %zu, number of matches: %d, bandwidth = %.3f GB/s\n",
           n, count, kStringLength / (1000000000.0 * timer.avg_sec(freq_ghz)));
  }
}

int main() {
  freq_ghz = measure_rdtsc_freq();
  printf("Kicking up TurboBoost\n");
  nano_sleep(2000000000, freq_ghz);
  printf("Starting work!\n");

  auto *string = new char[kStringLength];
  for (size_t i = 0; i < kStringLength; i++) {
    string[i] = '0' + (rand() % 10);
  }

  evaluate_hyperscan(string);

  return 0;
}
