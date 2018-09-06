#include <immintrin.h>
#include <stdlib.h>
#include <x86intrin.h>
#include "../common.h"

constexpr size_t kNum8BBlocks = MB(32);

void bench_avx512bw() {
  __m512i transitions[256];  // Garbage transisions are ok
  uint8_t data[8];

  struct timespec start;
  clock_gettime(CLOCK_REALTIME, &start);

  __m512i s;
  for (size_t iter = 0; iter < kNum8BBlocks; iter++) {
    uint8_t c1 = data[0];
    uint8_t c2 = data[1];
    uint8_t c3 = data[2];
    uint8_t c4 = data[3];
    uint8_t c5 = data[4];
    uint8_t c6 = data[5];
    uint8_t c7 = data[6];
    uint8_t c8 = data[7];
    s = _mm512_permutexvar_epi8(transitions[c1], s);  // This is what we need
    s = _mm512_shuffle_epi8(transitions[c2], s);
    s = _mm512_shuffle_epi8(transitions[c3], s);
    s = _mm512_shuffle_epi8(transitions[c4], s);
    s = _mm512_shuffle_epi8(transitions[c5], s);
    s = _mm512_shuffle_epi8(transitions[c6], s);
    s = _mm512_shuffle_epi8(transitions[c7], s);
    s = _mm512_shuffle_epi8(transitions[c8], s);
  }

  uint8_t *proof_of_work = reinterpret_cast<uint8_t *>(&s);
  double ns = ns_since(start);
  printf("ns per byte = %.2f, proof_of_work = %u\n", ns / (kNum8BBlocks * 8),
         *proof_of_work);
}

void bench_sse() {
  __m128i transitions[256];  // Garbage transisions are ok
  uint8_t data[8];

  struct timespec start;
  clock_gettime(CLOCK_REALTIME, &start);

  __m128i s;
  for (size_t iter = 0; iter < kNum8BBlocks; iter++) {
    uint8_t c1 = data[0];
    uint8_t c2 = data[1];
    uint8_t c3 = data[2];
    uint8_t c4 = data[3];
    uint8_t c5 = data[4];
    uint8_t c6 = data[5];
    uint8_t c7 = data[6];
    uint8_t c8 = data[7];
    s = _mm_shuffle_epi8(transitions[c1], s);
    s = _mm_shuffle_epi8(transitions[c2], s);
    s = _mm_shuffle_epi8(transitions[c3], s);
    s = _mm_shuffle_epi8(transitions[c4], s);
    s = _mm_shuffle_epi8(transitions[c5], s);
    s = _mm_shuffle_epi8(transitions[c6], s);
    s = _mm_shuffle_epi8(transitions[c7], s);
    s = _mm_shuffle_epi8(transitions[c8], s);
  }

  uint8_t *proof_of_work = reinterpret_cast<uint8_t *>(&s);
  double ns = ns_since(start);
  printf("ns per byte = %.2f, proof_of_work = %u\n", ns / (kNum8BBlocks * 8),
         *proof_of_work);
}

int main() {
  for (size_t i = 0; i < 10; i++) bench_sse();
  for (size_t i = 0; i < 10; i++) bench_avx512bw();
}
