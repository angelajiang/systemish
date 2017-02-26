/**
 * @file bench.cc
 * @brief What runs faster? A variable-sized loop that does more work, or a
 * fixed-size loop that does less work?
 */

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "city.h"

#define NUM_PKTS 30000000
#define N 8

/* Variable-sized loop that does less work */
void test_1(size_t n) {
  struct timespec start, end;
  clock_gettime(CLOCK_REALTIME, &start);

  size_t sum = 0;

  for (size_t i = 0; i < NUM_PKTS; i++) {
    for (size_t j = 0; j < N - n; j++) {
      sum += i + j + (i * j);
    }

    sum += CityHash64((char *)&sum, sizeof(size_t));
  }

  clock_gettime(CLOCK_REALTIME, &end);
  double seconds = (end.tv_sec - start.tv_sec) +
                   (double)(end.tv_nsec - start.tv_nsec) / 1000000000;
  double nanoseconds =
      (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);

  printf(
      "Variable-sized, smaller loop: "
      "Time = %f, time per packet iteration = %f ns, sum = %zu\n",
      seconds, nanoseconds / NUM_PKTS, sum);
}

/* Fixed-size loop that does more work */
void test_2() {
  struct timespec start, end;
  clock_gettime(CLOCK_REALTIME, &start);

  size_t sum = 0;

  for (size_t i = 0; i < NUM_PKTS; i++) {
    for (size_t j = 0; j < N; j++) {
      sum += i + j + (i * j);
    }

    sum += CityHash64((char *)&sum, sizeof(size_t));
  }

  clock_gettime(CLOCK_REALTIME, &end);
  double seconds = (end.tv_sec - start.tv_sec) +
                   (double)(end.tv_nsec - start.tv_nsec) / 1000000000;
  double nanoseconds =
      (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);

  printf(
      "Fixed-size, larger loop: "
      "Time = %f, time per packet iteration = %f ns, sum = %zu\n",
      seconds, nanoseconds / NUM_PKTS, sum);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: ./var_vs_fixed <number of iterations (< %d) to skip>\n", N);
    exit(-1);
  }

  assert(atoi(argv[1]) < N);

  size_t n = (size_t)atoi(argv[1]);

  for (size_t i = 0; i < 5; i++) {
    test_1(n);
    test_2();
    printf("\n");
  }

  return 0;
}
