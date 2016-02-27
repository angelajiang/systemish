/*
 * For small critical sections, spinlocks are the better choice.
 * Reason: pthread mutexes result in a context switch.
 */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<stdint.h>
#include<assert.h>
#include<errno.h>

#define NUM_THREADS 50

#define NUM_ROWS (32 * 1024 * 1024)
#define NUM_ROWS_ (NUM_ROWS - 1)

#define NANO 1000000000
#define M_1 (1 * 1024 * 1024)
#define M_1_ (M_1 - 1)

#define USE_SPINLOCK 1

struct row_t {
	pthread_spinlock_t spinlock;
	pthread_mutex_t mutex_lock;
	long long counter;
	long long pad[1];
};

struct thread_params {
	double *tput;
	int tid;
	struct row_t *row_arr;
};

void *thread_function(void *ptr);

int stick_this_thread_to_core(int core_id)
{
	int num_cores = 56;
	if (core_id < 0 || core_id >= num_cores)
		return EINVAL;

	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(core_id, &cpuset);

	pthread_t current_thread = pthread_self();    
	return pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
}

int main()
{
	int i;
	printf("%lu\n", sizeof(struct row_t));
	assert(sizeof(struct row_t) == 64);

	struct row_t *row_arr = malloc(NUM_ROWS * sizeof(*row_arr));

	for(i = 0; i < NUM_ROWS; i++) {
		pthread_spin_init(&row_arr[i].spinlock, 0);
		pthread_mutex_init(&row_arr[i].mutex_lock, NULL);
		row_arr[i].counter = 0;
	}
	
	double tput[NUM_THREADS];
	struct thread_params params[NUM_THREADS];
	pthread_t thread[NUM_THREADS];
	
	for(i = 0; i < NUM_THREADS; i++) {
		params[i].tid = i;
		params[i].row_arr = row_arr;
		tput[i] = 0.0;
		params[i].tput = tput;

		pthread_create(&thread[i], NULL, thread_function, &params[i]);
	}

	for(i = 0; i < NUM_THREADS; i++) {
		pthread_join(thread[i], NULL);
	}

	exit(0);
}

inline uint32_t
hrd_fastrand(uint64_t *seed)
{
    *seed = *seed * 1103515245 + 12345;
    return (uint32_t) (*seed >> 32);
}

void *thread_function(void *ptr)
{
	struct thread_params params = *(struct thread_params *) ptr;
	int tid = params.tid;
	stick_this_thread_to_core(tid);

	struct row_t *row_arr = params.row_arr;
	double *tput = params.tput;

	printf("Starting tid: %d\n", tid);

	int iter = 0, i;
	uint64_t seed = 0xdeadbeef;
	for(i = 0; i < tid * 1000000; i++) {
		hrd_fastrand(&seed);
	}

	struct timespec start, end;
	clock_gettime(CLOCK_REALTIME, &start);

	while(1) {
		if((iter & M_1_) == 0 && iter != 0) {
			clock_gettime(CLOCK_REALTIME, &end);
			double seconds = (end.tv_sec - start.tv_sec) + 
				((double) (end.tv_nsec - start.tv_nsec) / NANO);
			double my_tput = M_1 / seconds;
			tput[tid] = my_tput;

			if(tid == 0) {
				double total_tput = 0;
				int t_i;
				for(t_i = 0; t_i < NUM_THREADS; t_i++) {
					total_tput += tput[tid];
				}
				printf("Total throughput (%d threads) = %.2f Mops\n",
					NUM_THREADS, total_tput / 1000000);
			}

			clock_gettime(CLOCK_REALTIME, &start);
		}

		int row_i = hrd_fastrand(&seed) & NUM_ROWS_;

#if USE_SPINLOCK == 1
		pthread_spin_lock(&row_arr[row_i].spinlock);
		row_arr[row_i].counter++;
		pthread_spin_unlock(&row_arr[row_i].spinlock);
#else
		pthread_mutex_lock(&row_arr[row_i].mutex_lock);
		row_arr[row_i].counter++;
		pthread_mutex_unlock(&row_arr[row_i].mutex_lock);
#endif

		iter ++;
	}
}
