#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_OPS (1024 * 1024 * 128)

/// Check if changing the modulus affects performance. On Sandy Bridge, it
/// doesn't.
int main(int argc, char **argv) {
  assert(argc == 2);
  int modulo = atoi(argv[1]);
  printf("Computing modulo %d\n", modulo);

  struct timespec start, end;
  clock_gettime(CLOCK_REALTIME, &start);

  int sum = 0, i = 0;

  for (i = 0; i < NUM_OPS; i++) {
    sum += (i % modulo);
  }

  clock_gettime(CLOCK_REALTIME, &end);
  double seconds = (end.tv_sec - start.tv_sec) +
                   (double)(end.tv_nsec - start.tv_nsec) / 1000000000;
  double ns = (end.tv_sec - start.tv_sec) * 1000000000 +
              (double)(end.tv_nsec - start.tv_nsec);

  printf("Time = %f, sum = %d, ns per modulo = %.3f\n", seconds, sum,
         ns / NUM_OPS);

  return 0;
}
