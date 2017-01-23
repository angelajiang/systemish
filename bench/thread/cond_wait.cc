// Measure the time needed to start a thread
#include<stdio.h>
#include<unistd.h>
#include<stdint.h>
#include<time.h>
#include<thread>
#include<mutex>
#include<condition_variable>

#define ITERS 100

static uint64_t rdtsc() {
  uint64_t rax;
  uint64_t rdx;
  asm volatile("rdtsc" : "=a"(rax), "=d"(rdx));
  return (rdx << 32) | rax;
}

std::mutex mu;
std::condition_variable cv;
uint64_t start_tsc, end_tsc;

volatile bool waiting = false;

void thread_func() {
  while (1) {
    std::unique_lock<std::mutex> locker(mu);
    waiting = true;
    cv.wait(locker);
    printf("In consumer\n");
    locker.unlock();
  }
}

int main()
{
	uint64_t seed = 0xdeadbeef;

	int sum = 0;
	int iter;
	struct timespec start, end;
	double nanoseconds = 0;

  std::thread thread_1(thread_func);

  for (int i = 0; i < ITERS; i++) {
    while (!waiting) {
      // Do nothing
    }

    clock_gettime(CLOCK_REALTIME, &start);

    std::unique_lock<std::mutex> locker(mu);
    locker.unlock();
    cv.notify_one();

    clock_gettime(CLOCK_REALTIME, &end);

    nanoseconds += (end.tv_sec - start.tv_sec) * 1000000000 + 
      (end.tv_nsec - start.tv_nsec);
  }

	printf("Time per notify_one() = %f ns\n", nanoseconds / ITERS);

  thread_1.join();
}
