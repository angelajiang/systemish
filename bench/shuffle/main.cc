#include <immintrin.h>
#include <stdlib.h>
#include <x86intrin.h>
#include "../common.h"

constexpr size_t kNum8BBlocks = 16384;

int main() {
  typedef __m128 State;
  __m128i transitions[256];  // Garbage transisions are ok
  uint8_t data[8];

  struct timespec start;
  clock_gettime(CLOCK_REALTIME, &start);

  __m128i s;
  for (size_t i = 0; i < 8; i++) {
    uint8_t c1 = data[i + 0];
    uint8_t c2 = data[i + 1];
    uint8_t c3 = data[i + 2];
    uint8_t c4 = data[i + 3];
    uint8_t c5 = data[i + 4];
    uint8_t c6 = data[i + 5];
    uint8_t c7 = data[i + 6];
    uint8_t c8 = data[i + 7];
    s = _mm_shuffle_epi8(transitions[c1], s);
    s = _mm_shuffle_epi8(transitions[c2], s);
    s = _mm_shuffle_epi8(transitions[c3], s);
    s = _mm_shuffle_epi8(transitions[c4], s);
    s = _mm_shuffle_epi8(transitions[c5], s);
    s = _mm_shuffle_epi8(transitions[c6], s);
    s = _mm_shuffle_epi8(transitions[c7], s);
    s = _mm_shuffle_epi8(transitions[c8], s);
  }

  double ns = ns_since(start);
  printf("ns = %.2f\n", ns);
}
