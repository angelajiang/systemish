#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ITERS 2000000000

uint64_t seed = 0xdeadbeef;
inline uint32_t next_u32() {
  seed = seed * 1103515245 + 12345;
  return static_cast<uint32_t>(seed >> 32);
}

static uint32_t x = 123456789;
uint32_t xorshf32(void) {
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  return x;
}

int main() {
  struct timespec start, end;
  double nanoseconds;

  uint32_t sum = 0;

  // Fastrand
  clock_gettime(CLOCK_REALTIME, &start);
  for (int i = 0; i < ITERS; i++) {
    sum += next_u32();
  }
  clock_gettime(CLOCK_REALTIME, &end);

  nanoseconds =
      (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);

  printf("Time per fastrand = %f ns\n", nanoseconds / ITERS);

  // xorshf32
  clock_gettime(CLOCK_REALTIME, &start);
  for (int i = 0; i < ITERS; i++) {
    sum += xorshf32();
  }
  clock_gettime(CLOCK_REALTIME, &end);

  nanoseconds =
      (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);

  printf("Time per xorshf32 = %f ns\n", nanoseconds / ITERS);
}
