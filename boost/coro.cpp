#include <boost/coroutine/all.hpp>
#include <iostream>

using namespace boost::coroutines;

int i = 0;

boost::coroutines::symmetric_coroutine<void>::call_type* other_a = 0, * other_b = 0;

boost::coroutines::symmetric_coroutine<void>::call_type coro_a(
    [&](boost::coroutines::symmetric_coroutine<void>::yield_type& yield)
{
	while(i < 10) {
		printf("a yield to b, i = %d\n", i);
		i++;
		yield(*other_b);    // yield to coroutine coro_b
	}
});

boost::coroutines::symmetric_coroutine<void>::call_type coro_b(
    [&](boost::coroutines::symmetric_coroutine<void>::yield_type& yield)
{
	while(i < 10) {
		printf("b yield to a, i = %d\n", i);
		i++;
		yield(*other_a);    // yield to coroutine coro_b
	}
});

int main()
{
	other_a = &coro_a;
	other_b = &coro_b;
	coro_a(); // enter coroutine-fn of coro_a
}
