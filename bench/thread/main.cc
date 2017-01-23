// Measure the time needed to start a thread
#include<stdio.h>
#include<unistd.h>
#include<time.h>
#include<thread>

#define ITERS 100000

// Results on 2699-v3:
// Creating thread on same core: `taskset -c 0 ./main`: 2372 ns
// Creating thread on random core: `./main`: 7988 ns
static inline uint32_t hrd_fastrand(uint64_t *seed)
{
    *seed = *seed * 1103515245 + 12345;
    return (uint32_t) (*seed >> 32);
}

void thread_func() {
}

int main()
{
	uint64_t seed = 0xdeadbeef;

	int sum = 0;
	int iter;
	struct timespec start, end;
	double nanoseconds = 0;

  for (int i = 0; i < ITERS; i++) {
    clock_gettime(CLOCK_REALTIME, &start);
    std::thread thread_1(thread_func);
    clock_gettime(CLOCK_REALTIME, &end);

    nanoseconds += (end.tv_sec - start.tv_sec) * 1000000000 + 
      (end.tv_nsec - start.tv_nsec);

    thread_1.join();
  }

	printf("Time per thread creation = %f ns\n",
		nanoseconds / ITERS);
}
