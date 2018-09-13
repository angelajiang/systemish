#include <stdint.h>
#include <stdlib.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <set>

#include "../common.h"

void foo(FastRand &f, size_t &collision_count) {
  std::set<int> s;

  for (size_t i = 0; i < 10; i++) {
    int sample = static_cast<int>(f.next_u32());
    if (s.count(sample) > 0) {
      collision_count++;
      i--;
    } else {
      s.insert(sample);
    }
  }
}

int main() {
  FastRand f;
  size_t collision_count;

  struct timespec start;
  clock_gettime(CLOCK_REALTIME, &start);

  for (size_t i = 0; i < 1000; i++) {
    foo(f, collision_count);
  }

  printf("time = %.2f ns/iter collisions = %zu\n", ns_since(start) / 1000, collision_count);
}
