#include <assert.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "rte_memcpy.h"

static constexpr size_t kSize = 30000000;  // Array size in bytes

int main() {
  size_t iters = (30 * 1000000000ull) / kSize;  // 30 GB
  struct timespec start, end;

  uint8_t *A = reinterpret_cast<uint8_t *>(memalign(64, kSize));
  uint8_t *B = reinterpret_cast<uint8_t *>(memalign(64, kSize));

  // GLIBC
  printf("glibc memcpy\n");
  clock_gettime(CLOCK_REALTIME, &start);
  for (size_t i = 0; i < iters; i++) memcpy(A, B, kSize);

  clock_gettime(CLOCK_REALTIME, &end);
  double seconds = (end.tv_sec - start.tv_sec) +
                   (end.tv_nsec - start.tv_nsec) / 1000000000.0;
  printf("glibc GB/s = %.2f, A[0] = %u, seconds = %.2f\n",
         iters * kSize / (1000000000.0 * seconds), A[0], seconds);

  // DPDK
  printf("dpdk memcpy\n");
  clock_gettime(CLOCK_REALTIME, &start);
  for (size_t i = 0; i < iters; i++) rte_memcpy(A, B, kSize);

  clock_gettime(CLOCK_REALTIME, &end);
  seconds = (end.tv_sec - start.tv_sec) +
            (end.tv_nsec - start.tv_nsec) / 1000000000.0;
  printf("dpdk GB/s = %.2f, A[0] = %u, seconds = %.2f\n",
         iters * kSize / (1000000000.0 * seconds), A[0], seconds);

  return 0;
}
