#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>

#define ITERS 1000000 /* 1 million */
#define SIZE 200

static void memory_barrier() { asm volatile("" ::: "memory"); }

/// Return the TSC
static uint64_t rdtsc() {
  uint64_t rax;
  uint64_t rdx;
  asm volatile("rdtsc" : "=a"(rax), "=d"(rdx));
  return (rdx << 32) | rax;
}

int main() {

  std::vector<int> V;
  uint64_t sum = 0;

  /*
   * At each iteration, fill the vector, and then cut it off at a randomly
   * chosen point.
   */
  for (size_t iter = 0; iter < ITERS; iter++) {
    for (int i = 0; i < SIZE; i++) {
      V.push_back(i);
    }

    size_t n = SIZE - (rdtsc() % SIZE); /* Randomly choose n >= 1 */

    uint64_t tsc_start = rdtsc();
    memory_barrier();
    V.resize((size_t)n);
    memory_barrier();
    uint64_t tsc_end = rdtsc();
    sum += (tsc_end - tsc_start);

    size_t leftover = V.size();
    for (size_t i = 0; i < leftover; i++) {
      V.pop_back();
    }
  }

  printf("Time per random resize = %f cycles\n", (double)sum / ITERS);

  // Dummy iteration to calculate overhead of rdtsc
  sum = 0;
  for (size_t iter = 0; iter < ITERS; iter++) {
    for (int i = 0; i < SIZE; i++) {
      V.push_back(i);
    }

    uint64_t tsc_start = rdtsc();
    memory_barrier();
    uint64_t tsc_end = rdtsc();
    sum += (tsc_end - tsc_start);

    for (size_t i = 0; i < SIZE; i++) {
      V.pop_back();
    }
  }

  printf("rdtsc overhead = %f cycles\n", (double)sum / ITERS);
}
