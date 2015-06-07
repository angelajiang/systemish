#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

#define NUM_THREADS 4

struct thread_params {
	int tid;
	long long *shared_counter;
};

void *thread_func(void *ptr)
{
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

		long long cur_cntr = __sync_fetch_and_add(params->shared_counter, 1);

		iters ++;
	}
}

int main()
{
	int i;

	long long shared_counter = 0;
	struct thread_params param_arr[NUM_THREADS];

	pthread_t thread[NUM_THREADS];
	
	for(i = 0; i < NUM_THREADS; i++) {
		param_arr[i].tid = i;
		param_arr[i].shared_counter = &shared_counter;
		pthread_create(&thread[i], NULL, thread_func, &param_arr[i]);
	}

	for(i = 0; i < NUM_THREADS; i++) {
		pthread_join(thread[i], NULL);
	}

	exit(0);
}

