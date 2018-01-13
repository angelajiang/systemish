// Measure the overhead of clock_gettime() and rdtsc()
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static constexpr size_t kNumIters = (512 * 1024);

static inline size_t rdtsc() {
  unsigned low, high;
  unsigned long long val;
  asm volatile("rdtsc" : "=a"(low), "=d"(high));
  val = high;
  val = (val << 32) | low;
  return val;
}

// Measure overhead of clock_gettime with CLOCK_REALTIME
void test_clock_realtime() {
  struct timespec start, end;
  clock_gettime(CLOCK_REALTIME, &start);

  struct timespec temp;
  for (size_t i = 0; i < kNumIters; i++) {
    clock_gettime(CLOCK_REALTIME, &temp);
    clock_gettime(CLOCK_REALTIME, &temp);
    clock_gettime(CLOCK_REALTIME, &temp);
    clock_gettime(CLOCK_REALTIME, &temp);
    clock_gettime(CLOCK_REALTIME, &temp);
    clock_gettime(CLOCK_REALTIME, &temp);
    clock_gettime(CLOCK_REALTIME, &temp);
    clock_gettime(CLOCK_REALTIME, &temp);
    clock_gettime(CLOCK_REALTIME, &temp);
    clock_gettime(CLOCK_REALTIME, &temp);
  }

  clock_gettime(CLOCK_REALTIME, &end);
  double tot_ns = (end.tv_sec - start.tv_sec) * 1000000000.0 +
                  (end.tv_nsec - start.tv_nsec);

  printf("clock_gettime: Time per measurement = %.2f ns, final = %ld\n",
         tot_ns / (kNumIters * 10), temp.tv_nsec);
}

// Measure overhead of rdtsc
void test_rdtsc() {
  struct timespec start, end;
  size_t sum = 0;

  clock_gettime(CLOCK_REALTIME, &start);

  for (size_t i = 0; i < kNumIters; i++) {
    rdtsc();
    rdtsc();
    rdtsc();
    rdtsc();
    rdtsc();
    rdtsc();
    rdtsc();
    rdtsc();
    rdtsc();
    sum += rdtsc();
  }

  clock_gettime(CLOCK_REALTIME, &end);
  double tot_ns = (end.tv_sec - start.tv_sec) * 1000000000.0 +
                  (end.tv_nsec - start.tv_nsec);

  printf("RDTSC: Time per measurement = %.2f ns, sum = %lu\n",
         tot_ns / (kNumIters * 10), sum);
}

int main() {
  test_clock_realtime();
  test_clock_realtime();
  test_clock_realtime();
  test_clock_realtime();
  test_clock_realtime();
  test_rdtsc();
  test_rdtsc();
  test_rdtsc();
  test_rdtsc();
  test_rdtsc();
  return 0;
}
