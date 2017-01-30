#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <stdexcept>

#include "city.h"

#define BUF_SIZE 4
#define HASHES_PER_FUNC 2
#define ITERS 1000000
#define EXCEPTIONS_PER_MILLION_HASHES 0

//
// Benchmark the case where functions can throw exceptions
//
size_t exception_func(size_t initial_val) {
  size_t buf[BUF_SIZE];
  for (size_t i = 0; i < BUF_SIZE; i++) {
    buf[i] = initial_val;
  }

  for (size_t i = 0; i < HASHES_PER_FUNC; i++) {
    buf[0] = CityHash64((char *)buf, BUF_SIZE * sizeof(size_t));

    if (buf[0] % 1000000 < EXCEPTIONS_PER_MILLION_HASHES) {
      throw std::runtime_error("Runtime error");
    }
  }

  return buf[0];
}

void test_exception() {
  struct timespec start, end;
  size_t num_exceptions = 0;
  clock_gettime(CLOCK_REALTIME, &start);

  size_t ret = 0;
  for (size_t i = 0; i < ITERS; i++) {
    try {
      ret += exception_func(i);
    } catch (std::runtime_error e) {
      num_exceptions++;
    }
  }

  clock_gettime(CLOCK_REALTIME, &end);
  double nanoseconds =
      (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);

  std::cout << "Time per exception_func = " << nanoseconds / ITERS << " ns. "
            << "ret = " << ret << " num_exceptions = " << num_exceptions
            << std::endl;
}

//
// Benchmark the case where functions can throw exceptions
//
size_t no_exception_func(size_t initial_val) {
  size_t buf[BUF_SIZE];
  for (size_t i = 0; i < BUF_SIZE; i++) {
    buf[i] = initial_val;
  }

  for (size_t i = 0; i < HASHES_PER_FUNC; i++) {
    buf[0] = CityHash64((char *)buf, BUF_SIZE * sizeof(size_t));

    if (buf[0] % 1000000 < EXCEPTIONS_PER_MILLION_HASHES) {
      /* Keep the modulo operation, but don't throw an exception */
      buf[0]++;
    }
  }

  return buf[0];
}

void test_no_exception() {
  struct timespec start, end;
  clock_gettime(CLOCK_REALTIME, &start);

  size_t ret = 0;
  for (size_t i = 0; i < ITERS; i++) {
    ret += no_exception_func(i);
  }

  clock_gettime(CLOCK_REALTIME, &end);
  double nanoseconds =
      (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);

  std::cout << "Time per no_exception_func = " << nanoseconds / ITERS << " ns. "
            << "ret = " << ret << std::endl;
}

/// Trigger turboboost
void test_no_exception_warmup() {
  std::cout << "Warmup phase... " << std::endl;
  size_t ret = 0;
  for (size_t i = 0; i < 10 * ITERS; i++) {
    ret += no_exception_func(i);
  }

  std::cout << "Warmup phase... ret = " << ret << std::endl;
}

int main() {
  // Warmup the processor
  test_no_exception_warmup();

  test_exception();
  test_no_exception();
}
