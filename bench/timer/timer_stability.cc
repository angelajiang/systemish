// Check if rdtsc frequency is stable across TurboBoost frequency changes.
// This code assumes that rdtsc is always incremented at 2 GHz, which is the
// nominal frequency of E5-2683-v3.

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define NUM_ITERS (5) /* Total number of iterations */
#define RDTSC_GHZ (3.4)

static inline long long hrd_get_cycles() {
  unsigned low, high;
  unsigned long long val;
  asm volatile("rdtsc" : "=a"(low), "=d"(high));
  val = high;
  val = (val << 32) | low;
  return val;
}

void test() {
  struct timespec start, end;
  clock_gettime(CLOCK_REALTIME, &start);
  uint64_t rdtsc_start = hrd_get_cycles();

  /* Do some intense computation, followed by sleep -- multiple times */
  for (int loop = 0; loop < 2; loop++) {
    printf("Loop iter %d. Triggering turboboost.\n", loop);
    // Trigger turbo boost
    int sum = 5;
    for (int i = 0; i < 200000000; i++) {
      sum += i + (sum + i) * (i % sum);
    }

    printf("sum = %d, sleeping for 5 seconds.\n", sum);
    sleep(5);
  }

  clock_gettime(CLOCK_REALTIME, &end);
  size_t clock_ns =
      (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);
  size_t rdtsc_ns = (size_t)((hrd_get_cycles() - rdtsc_start) / RDTSC_GHZ);
  size_t delta_ns =
      (clock_ns > rdtsc_ns) ? (clock_ns - rdtsc_ns) : (rdtsc_ns - clock_ns);

  printf("clock_ns = %lu, rdtsc_ns = %lu, delta = %lu\n", clock_ns, rdtsc_ns,
         delta_ns);
}

int main() {
  for (int i = 0; i < NUM_ITERS; i++) {
    test();
  }
}
