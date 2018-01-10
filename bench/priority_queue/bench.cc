#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <queue>
#include <random>

#define USE_PRIORITY_QUEUE 1

static constexpr size_t kNumIters = 10000;

void test_perf(size_t num_items) {
#if USE_PRIORITY_QUEUE == 1
  std::priority_queue<double> pq;
#else
  std::queue<double> pq;
#endif

  std::uniform_real_distribution<double> unif(0, 100);
  std::default_random_engine re;
  struct timespec start, end;

  std::vector<double> input_vec;
  for (size_t i = 0; i < num_items; i++) {
    double sample = unif(re);
    input_vec.push_back(sample);
  }

  clock_gettime(CLOCK_REALTIME, &start);

  double sum = 1.0;
  for (size_t i = 0; i < kNumIters; i++) {
    for (size_t j = 0; j < num_items; j++) pq.push(input_vec[j]);
    for (size_t j = 0; j < num_items; j++) {
#if USE_PRIORITY_QUEUE == 1
      sum += pq.top();
#else
      sum += pq.front();
#endif
      pq.pop();
    }
  }

  clock_gettime(CLOCK_REALTIME, &end);
  ssize_t tot_ns =
      (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);

  printf("num_items = %zu: Time per operation = %.2f ns, sum = %.2f\n",
         num_items, static_cast<double>(tot_ns) / (kNumIters * num_items), sum);
}

int main() {
  for (size_t i = 1; i <= 100; i++) {
    test_perf(i * 10);
  }
  return 0;
}
