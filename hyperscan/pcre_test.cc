#include <pcre.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

void find_all_pcre(const char *pattern, const char *string) {
  int string_len = strlen(string);
  const char *error;
  int erroffset;

  pcre *re = pcre_compile(pattern, 0, &error, &erroffset, nullptr);
  if (re == nullptr) {
    fprintf(stderr, "ERROR: Unable to pcre_compile pattern %s\n", pattern);
    exit(-1);
  }

  int offset_vec[300];
  int ret = pcre_exec(re, nullptr, string, string_len, 0, 0, offset_vec,
                      300 * 3);

  if (ret == PCRE_ERROR_BADOPTION || ret == PCRE_ERROR_BADMAGIC) {
    fprintf(stderr, "ERROR: pcre_exec() failed. Pattern = %s\n", pattern);
    exit(-1);
  }

  printf("pattern = %s, string = %s, matches = %d\n", pattern, string, ret);;
}

int main() {
  find_all_pcre("ab/g", "aba");
  find_all_pcre("ab/g", "ababab");
  find_all_pcre("bb", "aaaaa");
  return 0;
}
