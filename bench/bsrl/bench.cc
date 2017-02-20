#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <stdexcept>

#define ITERS 500000000


/// Measure the time for bsrl
void test() {
  struct timespec start, end;
  clock_gettime(CLOCK_REALTIME, &start);

  size_t ret = 0;
  for (size_t i = 0; i < ITERS; i++) {
    int pos;
    asm ("bsrl %1, %0" 
      : "=r" (pos) 
      : "r" ((int)i << 1));

    ret += (size_t)pos;
  }

  clock_gettime(CLOCK_REALTIME, &end);
  double nanoseconds =
      (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);

  std::cout << "Time per bsrl = " << nanoseconds / ITERS << " ns. "
            << "ret = " << ret << std::endl;
}

/// For comparison, also measure the time for multiply-add
void test_2() {
  struct timespec start, end;
  clock_gettime(CLOCK_REALTIME, &start);

  size_t ret = 34;
  for (size_t i = 0; i < ITERS; i++) {
    ret += (size_t)(i * ret + 1);
  }

  clock_gettime(CLOCK_REALTIME, &end);
  double nanoseconds =
      (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);

  std::cout << "Time per MAC = " << nanoseconds / ITERS << " ns. "
            << "ret = " << ret << std::endl;
}


int main() {
  // Warmup the processor
  test();
  test();
  test();

  test_2();
  test_2();
  test_2();
  test_2();
  test_2();
}
