#include <hs/hs.h>
#include <pcre.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "timer.h"

static constexpr size_t kStringLength = 10000000;
double freq_ghz = 0;

static int match_count = 0;
static int event_handler(unsigned int, unsigned long long, unsigned long long,
                         unsigned int, void *) {
  match_count++;
  return 0;
}

void find_all_pcre(const char *pattern, const char *string) {
  int string_len = strlen(string);
  const char *error;
  int erroffset;

  pcre *re = pcre_compile(pattern, 0, &error, &erroffset, nullptr);
  if (re == nullptr) {
    fprintf(stderr, "ERROR: Unable to pcre_compile pattern %s\n", pattern);
    exit(-1);
  }

  int offset_vec[3];
  int ret = 0;
  int cur_offset = 0;

  while (true) {
    ret = pcre_exec(re, nullptr, string, string_len, cur_offset, 0, offset_vec,
                    3);
    if (ret == PCRE_ERROR_BADOPTION || ret == PCRE_ERROR_BADMAGIC) {
      fprintf(stderr, "ERROR: pcre_exec() failed. Pattern = %s\n", pattern);
      exit(-1);
    }

    if (ret == -1) break;

    match_count++;
    cur_offset = offset_vec[1] + 1;
  }
}

void evaluate_pcre(const char *string) {
  for (size_t n = 2; n <= 32; n++) {
    match_count = 0;

    // n is the number of (0|1) at the end of the regex
    std::string regex;
    regex += "(0|1)*1";
    for (size_t i = 0; i < n; i++) regex += "(0|1)";

    TscTimer timer;
    timer.start();
    find_all_pcre(regex.c_str(), string);
    timer.stop();

    printf("PCRE: n = %zu, number of matches: %d, bandwidth = %.3f GB/s\n", n,
           match_count,
           kStringLength / (1000000000.0 * timer.avg_sec(freq_ghz)));
  }
}

void find_all_hyperscan(const char *pattern, const char *string) {
  unsigned int string_len = strlen(string);
  hs_database_t *database;
  hs_compile_error_t *compile_err;
  if (hs_compile(pattern, 0, HS_MODE_BLOCK, nullptr, &database, &compile_err) !=
      HS_SUCCESS) {
    fprintf(stderr, "ERROR: Unable to compile pattern \"%s\": %s\n", pattern,
            compile_err->message);
    hs_free_compile_error(compile_err);
    exit(-1);
  }

  hs_scratch_t *scratch = nullptr;
  if (hs_alloc_scratch(database, &scratch) != HS_SUCCESS) {
    fprintf(stderr, "ERROR: Unable to allocate scratch space. Exiting.\n");
    hs_free_database(database);
    exit(-1);
  }

  if (hs_scan(database, string, string_len, 0, scratch, event_handler,
              nullptr) != HS_SUCCESS) {
    fprintf(stderr, "ERROR: Unable to scan input buffer. Exiting.\n");
    hs_free_scratch(scratch);
    exit(-1);
  }

  hs_free_scratch(scratch);
  hs_free_database(database);
}

void evaluate_hyperscan(const char *string) {
  for (size_t n = 2; n <= 32; n++) {
    match_count = 0;

    // n is the number of (0|1) at the end of the regex
    std::string regex;
    regex += "(0|1)*1";
    for (size_t i = 0; i < n; i++) regex += "(0|1)";

    TscTimer timer;
    timer.start();
    find_all_hyperscan(regex.c_str(), string);
    timer.stop();

    printf("HyperScan: n = %zu, number of matches: %d, bandwidth = %.3f GB/s\n",
           n, match_count,
           kStringLength / (1000000000.0 * timer.avg_sec(freq_ghz)));
  }
}

void hs_test(const char *regex, const char *string) {
  printf("regex = %s, string = %s\n", regex, string);
  find_all_hyperscan(regex, string);
  printf("count = %d\n", match_count);
  match_count = 0;
}

int main() {
  //hs_test("(a|b)*1(0|1)", "abababa10");
  freq_ghz = measure_rdtsc_freq();
  printf("Kicking up TurboBoost\n");
  nano_sleep(2000000000, freq_ghz);
  printf("Starting work!\n");

  auto *string = new char[kStringLength];
  for (size_t i = 0; i < kStringLength; i++) {
    string[i] = '0' + (rand() % 10);
  }

  evaluate_hyperscan(string);
  evaluate_pcre(string);

  return 0;
}
