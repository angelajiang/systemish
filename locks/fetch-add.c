/*
 * Multiple threads issue fetch-add on a shared array of counters in different
 * cachelines.
 */

#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<stdint.h>
#include<assert.h>

#define NUM_THREADS 14
#define NUM_COUNTERS 128
#define NUM_COUNTERS_ (NUM_COUNTERS - 1)

/* @cnt is in separate cachelines, regardless of @counter_arr's alignment */
struct counter {
	long long cnt;
	long long pad[7];
};

struct thread_params {
	int tid;
	struct counter *counter_arr;
};

inline uint32_t hrd_fastrand(uint64_t *seed)
{
    *seed = *seed * 1103515245 + 12345;
    return (uint32_t) (*seed >> 32);
}

void *thread_func(void *ptr)
{
	uint64_t seed = 0xdeadbeef;

	int iters = 0;
	struct timespec start, end;
	struct thread_params *params = (struct thread_params *) ptr;
	int tid = params->tid;
	printf("Starting tid: %d\n", tid);

	clock_gettime(CLOCK_REALTIME, &start);

	while(1) {

		if(iters == 10000000) {
			clock_gettime(CLOCK_REALTIME, &end);
			double seconds = (end.tv_sec - start.tv_sec) +
				(double) (end.tv_nsec - start.tv_nsec) / 1000000000;

			printf("Thread %d: %.2f M /s.\n", tid,
				10000000 / (seconds * 1000000));
	
			iters = 0;
			clock_gettime(CLOCK_REALTIME, &start);
		}

		int counter_i = hrd_fastrand(&seed) & NUM_COUNTERS_;
		__sync_fetch_and_add(&params->counter_arr[counter_i].cnt, 1);

		iters ++;
	}
}

int main()
{
	int i;

	struct counter *counter_arr = malloc(NUM_COUNTERS * sizeof(struct counter));
	assert(counter_arr != NULL);

	struct thread_params param_arr[NUM_THREADS];
	pthread_t thread[NUM_THREADS];
	
	for(i = 0; i < NUM_THREADS; i++) {
		param_arr[i].tid = i;
		param_arr[i].counter_arr = counter_arr;
		pthread_create(&thread[i], NULL, thread_func, &param_arr[i]);
	}

	for(i = 0; i < NUM_THREADS; i++) {
		pthread_join(thread[i], NULL);
	}

	exit(0);
}

