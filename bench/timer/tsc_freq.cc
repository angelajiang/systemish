#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static inline uint64_t rdtsc() {
  unsigned low, high;
  uint64_t val;
  asm volatile("rdtsc" : "=a"(low), "=d"(high));
  val = high;
  val = (val << 32) | low;
  return val;
}

/**
 * @brief Measure rdtsc frequency by comparing cycles against real-time
 * nanoseconds.
 */
double get_tsc_freq() {
  struct timespec start, end;
  clock_gettime(CLOCK_REALTIME, &start);
  uint64_t rdtsc_start = rdtsc();

  /*
   * Do not change this loop! The hardcoded value below depends on this loop
   * and prevents it from being optimized out.
   */
  uint64_t sum = 5;
  for (uint64_t i = 0; i < 1000000; i++) {
    sum += i + (sum + i) * (i % sum);
  }

  if (sum != 13580802877818827968ull) {
    fprintf(stderr, "eRPC: FATAL. Failed in rdtsc frequency measurement.");
    exit(-1);
  }

  clock_gettime(CLOCK_REALTIME, &end);
  uint64_t clock_ns =
      (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);
  uint64_t rdtsc_cycles = rdtsc() - rdtsc_start;

  return (double)rdtsc_cycles / clock_ns;
}

int main() { printf("freq = %.2f\n", get_tsc_freq()); }
