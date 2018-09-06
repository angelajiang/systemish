#include <stdint.h>
#include <stdlib.h>

#include <assert.h>
#include <hdr/hdr_histogram.h>
#include <stdio.h>
#include <string.h>

#include "../common.h"

static constexpr size_t kMaxValue = 100000;
static constexpr size_t kMinValue = 1;
static constexpr size_t kDecimalPlaces = 1;
static constexpr size_t kIterations = MB(100);

int main() {
  int result = -1;

  struct hdr_histogram* histogram;
  result =
      hdr_init(static_cast<int64_t>(kMinValue), static_cast<int64_t>(kMaxValue),
               kDecimalPlaces, &histogram);
  assert(result == 0);

  printf("bytes used by histogram = %zu\n", hdr_get_memory_size(histogram));

  struct timespec start;
  clock_gettime(CLOCK_REALTIME, &start);

  for (size_t i = 0; i < 100; i++) {
    for (size_t j = 0; j < kIterations; j++) {
      hdr_record_value(histogram, static_cast<int64_t>(j));
    }

    printf("Million updates/sec: %.2f\n",
           kIterations / (sec_since(start) * 1000000));
    clock_gettime(CLOCK_REALTIME, &start);
  }
  return 0;
}
