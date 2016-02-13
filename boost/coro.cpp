#include <boost/coroutine/all.hpp>
#include <iostream>
#include <time.h>

#define NUM_SWITCHES 200000000	/* 500 million switches */
#define NUM_THREADS 8
#define NUM_THREADS_ 7

#define FPU_ENABLE 1
#define FPU_DISABLE 0

using namespace boost::coroutines;
int switch_i = 0, thread_id = 0;

boost::coroutines::symmetric_coroutine<void>::call_type *thread_arr[NUM_THREADS];

/*
 * The function executed by each coroutine. @switch_i represents the number of
 * switches executed until now.
 */
void thread_func(boost::coroutines::symmetric_coroutine<void>::yield_type& yield)
{
	while(switch_i < NUM_SWITCHES) {
		switch_i++;
		thread_id = (thread_id + 1) & (NUM_THREADS - 1);
		yield(*thread_arr[thread_id]);
	}
}

/*
 * Test coroutine switching performance with FPU register saving enabled or
 * disabled.
 */
void test(int is_fpu_enabled)
{
	struct timespec timer_start, timer_end;
	boost::coroutines::flag_fpu_t fpu_flag = (is_fpu_enabled == FPU_ENABLE) ?
		boost::coroutines::fpu_preserved : boost::coroutines::fpu_not_preserved;

	/* Create @NUM_THREADS coroutines */
	for(int thr_i = 0; thr_i < NUM_THREADS; thr_i++) {
		thread_arr[thr_i] = new boost::coroutines::symmetric_coroutine<void>::call_type(
			thread_func, boost::coroutines::attributes(fpu_flag));
	}

	clock_gettime(CLOCK_REALTIME, &timer_start);

	/* Launch the 1st coroutine; this calls other coroutines later. */
	(*thread_arr[0])();

	clock_gettime(CLOCK_REALTIME, &timer_end);
	assert(switch_i == NUM_SWITCHES);

	double ns = (timer_end.tv_sec - timer_start.tv_sec) * 1000000000 +
		(double) (timer_end.tv_nsec - timer_start.tv_nsec);
	printf("main: Time = %.2f ns, context switch time = %.2f ns, FPU enabled = %s\n",
		ns, ns / NUM_SWITCHES, is_fpu_enabled == FPU_ENABLE ? "yes" : "no");

	/* Destroy the coroutines. */
	for(int thr_i = 0; thr_i < NUM_THREADS; thr_i++) {
		delete thread_arr[thr_i];
	}
}

int main()
{
	/* Warm up the core (useful if power saving or turbo-boost is enabled) */
	printf("Warming up\n");
	int sum = 0;
	for(int i = 0; i < 1000000000; i++) {
		sum += i * i;
	}

	printf("Testing with FPU enabled\n");
	test(FPU_ENABLE);

	printf("Testing with FPU disabled\n");
	switch_i = 0; 	/* Restart switches */
	test(FPU_DISABLE);

	printf("sum = %d\n", sum);
}
