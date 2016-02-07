/*
 * Compare the overhead of passing around thread state with function calls vs
 * using TLS for thread state.
 */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<getopt.h>
#include<assert.h>

#define ARR_SZ 63	/* A weird size to check for cacheline bouncing */
#define ITERS 20000000 /* 20 million */

void *thread_func_tls(void *ptr);
void *thread_func_no_tls(void *ptr);

__thread int tls_var_1 __attribute__ ((aligned(64)));
__thread int tls_var_2 __attribute__ ((aligned(64)));
__thread long long tls_arr_3[ARR_SZ] __attribute__ ((aligned(64)));
__thread int tls_arr_4[ARR_SZ] __attribute__ ((aligned(64)));
__thread long long tls_arr_5[ARR_SZ] __attribute__ ((aligned(64)));

int main(int argc, char **argv)
{
	int i, c;
	int num_threads = -1, use_tls = -1;
	static struct option opts[] = {
		{ .name = "num-threads",		.has_arg = 1, .val = 't' },
		{ .name = "use-tls",			.has_arg = 1, .val = 'u' },
		{ 0 }
	};

	/* Parse and check arguments */
	while(1) {
		c = getopt_long(argc, argv, "use-tls", opts, NULL);
		if(c == -1) {
			break;
		}
		switch (c) {
			case 't':
				num_threads = atoi(optarg);
				break;
			case 'u':
				use_tls = atoi(optarg);
				break;
			default:
				printf("Invalid argument %d\n", c);
				assert(0);
		}
	}

	assert(num_threads >= 0 && num_threads <= 56);
	assert(use_tls == 0 || use_tls == 1);
	
	pthread_t *threads = malloc(num_threads * sizeof(*threads));
	int *tid = malloc(num_threads * sizeof(*tid));

	for(i = 0; i < num_threads; i++) {
		tid[i] = i;
		if(use_tls == 1) {
			pthread_create(&threads[i], NULL, thread_func_tls, &tid[i]);
		} else {
			pthread_create(&threads[i], NULL, thread_func_no_tls, &tid[i]);
		}
	}

	for(i = 0; i < num_threads; i++) {
		pthread_join(threads[i], NULL);
	}

	exit(0);
}

/* TLS version */

/*
 * Force the function to be not inlined, which is what we expect to happen in
 * a real application.
 */
void __attribute__ ((noinline)) do_op_tls()
{
	int j;

	tls_var_1++;
	tls_var_2++;
	for(j = 2; j < ARR_SZ; j++) {
		tls_arr_3[j]++;
		tls_arr_4[j] += tls_arr_3[j - 1];	/* j starts from 2 */
		tls_arr_5[j] += tls_arr_4[j - 2];	/* j starts from 2 */
	}
}

void *thread_func_tls(void *ptr)
{
	int tid = *(int *) ptr;
	assert(tid >= 0 && tid <= 56);
	printf("Starting TLS thread %d, tls_var_1 addr = %p\n", tid, &tls_var_1);

	tls_var_1 = 0;	
	tls_var_2 = 0;
	memset(tls_arr_3, 0, ARR_SZ * sizeof(long long));
	memset(tls_arr_4, 0, ARR_SZ * sizeof(int));
	memset(tls_arr_5, 0, ARR_SZ * sizeof(long long));

	struct timespec start, end;
	clock_gettime(CLOCK_REALTIME, &start);

	int i = 0, j;
	for(i = 0; i < ITERS; i++) {
		do_op_tls();
	}

	clock_gettime(CLOCK_REALTIME, &end);
	double seconds = (end.tv_sec - start.tv_sec) + 
		(double) (end.tv_nsec - start.tv_nsec) / 1000000000;
	printf("main: Thread %d, time = %.2f seconds, sum = %lld\n", tid, seconds,
		tls_var_1 + tls_var_2 + tls_arr_5[ARR_SZ - 1]);
}

/* No-TLS version */

/*
 * Force the function to be not inlined, which is what we expect to happen in
 * a real application.
 */
void __attribute__ ((noinline)) do_op_no_tls(int *var_1, int *var_2,
	long long *arr_3, int *arr_4, long long *arr_5)
{
	int j;
	*var_1++;
	*var_2++;
	for(j = 2; j < ARR_SZ; j++) {
		arr_3[j]++;
		arr_4[j] += arr_3[j - 1];	/* j starts from 2 */
		arr_5[j] += arr_4[j - 2];	/* j starts from 2 */
	}
}
	
void *thread_func_no_tls(void *ptr)
{
	int tid = *(int *) ptr;
	assert(tid >= 0 && tid <= 56);
	printf("Starting non-TLS thread %d\n", tid);

	/* Declare variables on the thread stack */
	int var_1;
	int var_2;
	long long arr_3[ARR_SZ];
	int arr_4[ARR_SZ];
	long long arr_5[ARR_SZ];

	var_1 = 0;	
	var_2 = 0;
	memset(arr_3, 0, ARR_SZ * sizeof(long long));
	memset(arr_4, 0, ARR_SZ * sizeof(int));
	memset(arr_5, 0, ARR_SZ * sizeof(long long));

	struct timespec start, end;
	clock_gettime(CLOCK_REALTIME, &start);

	int i = 0;
	for(i = 0; i < ITERS; i++) {
		do_op_no_tls(&var_1, &var_2, arr_3, arr_4, arr_5);
	}

	clock_gettime(CLOCK_REALTIME, &end);
	double seconds = (end.tv_sec - start.tv_sec) + 
		(double) (end.tv_nsec - start.tv_nsec) / 1000000000;
	printf("main: Thread %d, time = %.2f seconds, sum = %lld\n", tid, seconds,
		var_1 + var_2 + arr_5[ARR_SZ - 1]);
}
