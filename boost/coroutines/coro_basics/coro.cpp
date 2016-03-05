#include <boost/coroutine/all.hpp>
#include <iostream>
#include <time.h>

#define NUM_THREADS 8
#define NUM_THREADS_ 7

#define NUM_SWITCHES_PER_THREAD 10000000	/* 10 million */
#define NUM_SWITCHES (NUM_THREADS * NUM_SWITCHES_PER_THREAD)

using namespace boost::coroutines;
int coro_id = 0;

boost::coroutines::symmetric_coroutine<void>::call_type *coro_arr[NUM_THREADS];

/*
 * The function executed by each coroutine. @switch_i represents the number of
 * switches executed until now.
 */
void coro_func(boost::coroutines::symmetric_coroutine<void>::yield_type& yield)
{
	int switch_i = 0;

	int var_1 = 0, var_2 = 0, var_3 = 0;
	long long buf_1[1024] = {0};
	long long buf_2[1024] = {0};
	double fpu_checker = 0.0;

	while(switch_i < NUM_SWITCHES_PER_THREAD) {
		var_1++;
		var_2 += var_1;
	
		buf_1[var_2 & 1023]++;
		buf_2[var_2 & 511]++;
		fpu_checker += buf_2[50];
		var_3 += buf_1[111];

		switch_i++;

		coro_id = (coro_id + 1) & (NUM_THREADS - 1);
		yield(*coro_arr[coro_id]);
	}

	printf("var_1 = %d, var_2 = %d, buf_1[1023] = %lld, buf_2[511] = %lld\n "
		"FPU checker = %f, var_3 = %d\n",
		var_1, var_2, buf_1[1023], buf_2[1023], fpu_checker, var_3);
}

/* Test coroutine switching performance. */
void test()
{
	struct timespec timer_start, timer_end;
	boost::coroutines::flag_fpu_t fpu_flag =
		boost::coroutines::fpu_not_preserved;

	/* Create @NUM_THREADS coroutines */
	for(int thr_i = 0; thr_i < NUM_THREADS; thr_i++) {
		coro_arr[thr_i] =
			new boost::coroutines::symmetric_coroutine<void>::call_type(
				coro_func, boost::coroutines::attributes(fpu_flag));
	}

	clock_gettime(CLOCK_REALTIME, &timer_start);

	/* Launch the 1st coroutine; this calls other coroutines later. */
	(*coro_arr[0])();

	clock_gettime(CLOCK_REALTIME, &timer_end);
	printf("Thread ID = %d\n", coro_id);	/* Prevent optimization */

	double ns = (timer_end.tv_sec - timer_start.tv_sec) * 1000000000 +
		(double) (timer_end.tv_nsec - timer_start.tv_nsec);
	printf("main: Time = %.2f ns, context switch time = %.2f ns, ",
		ns, ns / NUM_SWITCHES);

	/* Destroy the coroutines. */
	for(int thr_i = 0; thr_i < NUM_THREADS; thr_i++) {
		delete coro_arr[thr_i];
	}
}

int main()
{
	/* Warm up the core (useful if power saving or Turbo Boost is enabled) */
	printf("Warming up\n");
	int sum = 0;
	for(int i = 0; i < 1000000000; i++) {
		sum += i * i;
	}

	test();

	printf("sum = %d\n", sum);
}
