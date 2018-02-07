#include <hs/hs.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

static constexpr size_t kStringLength = 100000;
static constexpr size_t kStateExposionN = 32;

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

int main() {
  auto *string = new char[kStringLength];
  for (size_t i = 0; i < kStringLength; i++) {
    string[i] = '0' + (rand() % 2);
  }

  std::string regex;
  regex += "(0|1)*1";
  for (size_t i = 0; i < kStateExposionN; i++) {
    regex += "(0|1)";
  }

  if (kStringLength < 50) {
    printf("string = %s, regex = %s\n", string, regex.c_str());
  }

  hs_find_all(regex.c_str(), string);

  printf("Number of matches: %d\n", count);
  return 0;
}
