#include <papi.h>
#include <stdio.h>
#include <stdlib.h>

#define PAPI_COUNTER PAPI_L2_DCA
//#define PAPI_COUNTER PAPI_TOT_INS

double your_slow_code() {
  int i;
  double tmp = 1.1;

  for (i = 1; i < 100000000; i++) {
    tmp = (tmp + 100) / i;
  }

  return tmp;
}

void papi_start(void) {
  int counter_type = PAPI_COUNTER;
  if (PAPI_start_counters(&counter_type, 1) != PAPI_OK) {
    fprintf(stderr, "Error initializing counter.\n");
    exit(-1);
  }
}

void papi_mark(void) {
  long long final_count;
  if (PAPI_read_counters(&final_count, 1) != PAPI_OK) {
    fprintf(stderr, "Error reading instruction count\n");
    exit(-1);
  } else {
    printf("Final counter = %lld\n", final_count);
  }
}

int main() {
  papi_start();
  double sum = your_slow_code();
  papi_mark();

  printf("Sum = %f\n", sum);
}
