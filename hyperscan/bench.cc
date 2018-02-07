#include <hs/hs.h>
#include <stdio.h>
#include <string.h>

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
  hs_find_all("aa", "aaa");
  printf("hs matches: %d\n", count);

  return 0;
}
