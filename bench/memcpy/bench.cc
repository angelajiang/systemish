#include <assert.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "rte_memcpy.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: ./main <size>\n");
    exit(0);
  }

  size_t size = static_cast<size_t>(atoi(argv[1]));
  //size = 1024;

  size_t iters = (10 * 1000000000ull) / size;  // 10 GB
  struct timespec start, end;

  uint8_t *A = reinterpret_cast<uint8_t *>(memalign(64, size));
  uint8_t *B = reinterpret_cast<uint8_t *>(memalign(64, size));

  // GLIBC
  printf("glibc memcpy\n");
  clock_gettime(CLOCK_REALTIME, &start);
  for (size_t i = 0; i < iters; i++) memcpy(A, B, size);

  clock_gettime(CLOCK_REALTIME, &end);
  double seconds = (end.tv_sec - start.tv_sec) +
                   (end.tv_nsec - start.tv_nsec) / 1000000000.0;
  double ns = seconds * 1000000000.0;
  printf("glibc GB/s = %.2f, iters/ns = %.2f, A[1] = %u, seconds = %.2f\n",
         iters * size / ns, iters / ns, A[0], seconds);

  // DPDK
  printf("dpdk memcpy\n");
  clock_gettime(CLOCK_REALTIME, &start);
  for (size_t i = 0; i < iters; i++) rte_memcpy(A, B, size);

  clock_gettime(CLOCK_REALTIME, &end);
  seconds = (end.tv_sec - start.tv_sec) +
            (end.tv_nsec - start.tv_nsec) / 1000000000.0;
  ns = seconds * 1000000000.0;
  printf("dpdk GB/s = %.2f, iters/ns = %.2f, A[0] = %u, seconds = %.2f\n",
         iters * size / ns, iters / ns, A[0], seconds);

  return 0;
}
