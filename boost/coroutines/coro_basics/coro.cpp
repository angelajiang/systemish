#include <assert.h>
#include <boost/bind.hpp>
#include <boost/coroutine/all.hpp>
#include <iostream>
#include <chrono>

#define NUM_CORO 8
#define NUM_CORO_ 7

#define NUM_SWITCHES_PER_CORO 10000000	/* 10 million */
#define NUM_SWITCHES (NUM_CORO * NUM_SWITCHES_PER_CORO)

#define DEBUG 1

using namespace std;
using namespace std::chrono;
using namespace boost::coroutines;

/* Check if we can unwind stack and debug inside coroutines */
void buggy_func()
{
	assert(false);
}

/*
 * The function executed by each coroutine. @switch_i represents the number of
 * switches executed until now.
 */
void coro_func(symmetric_coroutine<void>::yield_type& yield,
	int coro_id, symmetric_coroutine<void>::call_type *coro_arr)
{
	cout << "coro_func: In coroutine " << coro_id << ", coro_arr " <<
		coro_arr << endl;
	int switch_i = 0;

	int var_1 = 0, var_2 = 0, var_3 = 0;
	long long buf_1[1024] = {0};
	long long buf_2[1024] = {0};
	double fpu_checker = 0.0;

	while(switch_i < NUM_SWITCHES_PER_CORO) {
#if DEBUG == 1
		if(switch_i == 1 && coro_id == 4) {
			buggy_func();
		}
#endif

		var_1++;
		var_2 += var_1;
	
		buf_1[var_2 & 1023]++;
		buf_2[var_2 & 511]++;
		fpu_checker += buf_2[50];
		var_3 += buf_1[111];

		switch_i++;

		int next_coro_id = (coro_id + 1) & (NUM_CORO - 1);
		yield(coro_arr[next_coro_id]);
	}

	printf("coro %d: var_1 = %d, var_2 = %d, buf_1[1023] = %lld, "
		"buf_2[511] = %lld\n"
		"FPU checker = %f, var_3 = %d\n", coro_id,
		var_1, var_2, buf_1[1023], buf_2[1023], fpu_checker, var_3);
}

/* Test coroutine switching performance. */
void test()
{
	symmetric_coroutine<void>::call_type coro_arr[NUM_CORO];

	auto timer_start = high_resolution_clock::now();
	flag_fpu_t fpu_flag = fpu_not_preserved;

	/* Create @NUM_CORO coroutines */
	for(int coro_i = 0; coro_i < NUM_CORO; coro_i++) {
		cout << "test: Init coro " << coro_i << endl;
		/*
		 * bind(coro_func, _1, coro_i, coro_arr) creates a function pointer
		 * with the 1st argument of coro_func unbound, and the remaining two
		 * arguments bound to coro_i and coro_arr respectively.
		 *
		 * Do not create a temporary loop variable to store the bound functor.
		 */
		coro_arr[coro_i] = symmetric_coroutine<void>::call_type(
			bind(coro_func, _1, coro_i, coro_arr), attributes(fpu_flag));
	}

	/* Launch the 1st coroutine; this calls other coroutines later. */
	(coro_arr[0])();

	auto timer_end = high_resolution_clock::now();

	auto ns = duration_cast<nanoseconds>(timer_end - timer_start).count();
	cout << "main: Time =  " << ns << ", context switch time = " <<
		ns / NUM_SWITCHES << " ns" << endl;
}

int main()
{
	/* Warm up the core (useful if power saving or Turbo Boost is enabled) */
	printf("Warming up\n");
	int sum = 0;
	for(int i = 0; i < 100000000; i++) {
		sum += i * i;
	}

	test();

	printf("sum = %d\n", sum);	/* Prevent warmup optimization */
}
