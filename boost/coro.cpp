#include <boost/coroutine/all.hpp>
#include <iostream>
#include <time.h>

#define MAX 10000000

using namespace boost::coroutines;
int i = 0;


boost::coroutines::symmetric_coroutine<void>::call_type* other_a = 0, * other_b = 0;

boost::coroutines::symmetric_coroutine<void>::call_type coro_a(
    [&](boost::coroutines::symmetric_coroutine<void>::yield_type& yield)
{
	while(i < MAX) {
		i++;
		yield(*other_b);    // yield to coroutine coro_b
	}
});

boost::coroutines::symmetric_coroutine<void>::call_type coro_b(
    [&](boost::coroutines::symmetric_coroutine<void>::yield_type& yield)
{
	clock_gettime(CLOCK_REALTIME, &timer_start);

	while(i < MAX) {
		i++;
		yield(*other_a);    // yield to coroutine coro_b
	}
});

int main()
{
	struct timespec timer_start, timer_end;

	other_a = &coro_a;
	other_b = &coro_b;

	coro_a(); // enter coroutine-fn of coro_a

	clock_gettime(CLOCK_REALTIME, &timer_end);
	assert(i == MAX);

	double ns = (timer_end.tv_sec - timer_start.tv_sec) * 1000000000 +
		(double) (timer_end.tv_nsec - timer_start.tv_nsec);
	printf("main: Time = %.2f ns, context switch time = %.2f ns\n",
		ns, ns / MAX);

}
