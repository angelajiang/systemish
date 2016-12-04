// Measure the overhead of clock_gettime() and rdtsc()
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_ITERS (32 * 1024 * 1024) /* Total number of iterations */

/* Timer is sampled ever @STEPPING iterations */
#define STEPPING 7
static_assert(STEPPING == 7 || STEPPING == 15 || STEPPING == 31, "");

static inline long long hrd_get_cycles() {
  unsigned low, high;
  unsigned long long val;
  asm volatile("rdtsc" : "=a"(low), "=d"(high));
  val = high;
  val = (val << 32) | low;
  return val;
}

/* Measure overhead of clock_gettime with CLOCK_REALTIME */
void test_clock_realtime() {
  struct timespec start, end;
  size_t sum_ns = 0;

  clock_gettime(CLOCK_REALTIME, &start);

  for (int i = 0; i < NUM_ITERS; i++) {
    if ((i & STEPPING) == STEPPING) {
      struct timespec iter_start, iter_end;
      clock_gettime(CLOCK_REALTIME, &iter_start);
      clock_gettime(CLOCK_REALTIME, &iter_end);

      size_t iter_ns = (iter_end.tv_sec - iter_start.tv_sec) * 1000000000 +
                       (iter_end.tv_nsec - iter_start.tv_nsec);

      sum_ns += iter_ns;
    }
  }

  clock_gettime(CLOCK_REALTIME, &end);
  size_t tot_ns =
      (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);

  printf("CLOCK_GETTIME: Time per measurement = %.2f ns, sum = %lu\n",
         (double)tot_ns / NUM_ITERS, sum_ns);
}

/* Measure overhead of rdtsc */
void test_rdtsc() {
  struct timespec start, end;
  size_t sum_ns = 0;

  clock_gettime(CLOCK_REALTIME, &start);

  for (int i = 0; i < NUM_ITERS; i++) {
    if ((i & STEPPING) == STEPPING) {
      long long iter_start = hrd_get_cycles();
      long long iter_end = hrd_get_cycles();

      size_t iter_ns = iter_end - iter_start;
      sum_ns += iter_ns;
    }
  }

  clock_gettime(CLOCK_REALTIME, &end);
  size_t tot_ns =
      (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);

  printf("RDTSC: Time per measurement = %.2f ns, sum = %lu\n",
         (double)tot_ns / NUM_ITERS, sum_ns);
}

int main(int argc, char **argv) {
  test_clock_realtime();
  test_rdtsc();

  return 0;
}
