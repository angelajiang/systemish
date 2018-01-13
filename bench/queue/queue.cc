#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <queue>

static constexpr size_t kNumIters = (8 * 1024 * 1024);
static constexpr size_t kQueueInitialSize = 1024;

void test_queue() {
  std::queue<size_t> queue;
  for (size_t i = 0; i < kQueueInitialSize; i++) queue.push(i);
  struct timespec start, end;
  clock_gettime(CLOCK_REALTIME, &start);

  for (size_t i = 0; i < kNumIters; i++) {
    queue.push(i);
    queue.push(i);
    queue.push(i);
    queue.push(i);
    queue.push(i);

    queue.pop();
    queue.pop();
    queue.pop();
    queue.pop();
    queue.pop();
  }

  clock_gettime(CLOCK_REALTIME, &end);
  double tot_ns = (end.tv_sec - start.tv_sec) * 1000000000.0 +
                  (end.tv_nsec - start.tv_nsec);

  printf("clock_gettime: Time per measurement = %.2f ns, final = %zu\n",
         tot_ns / (kNumIters * 10), queue.front());
}

void test_queue_2() {
  std::queue<size_t> queue;
  for (size_t i = 0; i < kQueueInitialSize; i++) queue.push(i);
  struct timespec start, end;
  clock_gettime(CLOCK_REALTIME, &start);

  for (size_t i = 0; i < kNumIters; i++) {
    queue.push(i);
    queue.pop();

    queue.push(i);
    queue.pop();

    queue.push(i);
    queue.pop();

    queue.push(i);
    queue.pop();

    queue.push(i);
    queue.pop();
  }

  clock_gettime(CLOCK_REALTIME, &end);
  double tot_ns = (end.tv_sec - start.tv_sec) * 1000000000.0 +
                  (end.tv_nsec - start.tv_nsec);

  printf("clock_gettime: Time per measurement = %.2f ns, final = %zu\n",
         tot_ns / (kNumIters * 10), queue.front());
}

int main() {
  test_queue();
  test_queue();
  test_queue();
  test_queue();
  test_queue();

  test_queue_2();
  test_queue_2();
  test_queue_2();
  test_queue_2();
  test_queue_2();

  return 0;
}
