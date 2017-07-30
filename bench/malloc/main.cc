#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <array>
#include <chrono>

#define N 10000
#define SIZE 64

int main() {
  size_t sum = 1;
  std::chrono::high_resolution_clock timer;

  printf("Starting warmup\n");
  for (size_t i = 0; i < 1000000000; i++) {
    sum += sum * (i + sum);
  }
  printf("Warmup done, sum = %zu \n", sum);

  std::array<void *, N> arr;

  auto start = timer.now();
  for (size_t i = 0; i < N; i++) {
    arr[i] = malloc(SIZE);
    memset(static_cast<uint8_t *>(arr[i]), 0, SIZE);
  }

  for (size_t i = 0; i < N; i++) free(arr[i]);

  auto end = timer.now();
  double ns =
      std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  printf("Time per malloc-free = %.2f ns\n", ns / N);
}
