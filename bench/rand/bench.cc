#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../common.h"
#include "pcg/pcg_random.hpp"

static constexpr size_t kIters = MB(200);

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
  size_t sum = 0;

  // Fastrand
  clock_gettime(CLOCK_REALTIME, &start);
  for (size_t i = 0; i < kIters; i++) sum += next_u32();
  clock_gettime(CLOCK_REALTIME, &end);

  double ns = ns_since(start);
  printf("Time per fastrand = %f ns, sum = %zu\n", ns / kIters, sum);

  // xorshf32
  clock_gettime(CLOCK_REALTIME, &start);
  for (size_t i = 0; i < kIters; i++) sum += xorshf32();
  ns = ns_since(start);
  printf("Time per xorshf32 = %f ns, sum = %zu\n", ns / kIters, sum);

  // pcg
  clock_gettime(CLOCK_REALTIME, &start);
  pcg64_fast pcg(pcg_extras::seed_seq_from<std::random_device>{});
  for (size_t i = 0; i < kIters; i++) sum += pcg();
  ns = ns_since(start);
  printf("Time per pcg64 = %f ns, sum = %zu\n", ns / kIters, sum);
}
