#include <boost/pool/object_pool.hpp>
#include <boost/timer/timer.hpp>
#include <iostream>

#define NUM_ITERS 20000000
#define SIZE 65	/* Size of the struct */
#define NUM_MSG 64

#define USE_MALLOC 1

struct msg_t {
	char A[SIZE];
};

int main()
{
	struct msg_t *msg_arr[NUM_MSG];
	boost::object_pool<struct msg_t> pool{64, 0};
	boost::timer::cpu_timer timer;
	int counter = 0;

#if USE_MALLOC == 1
	/* Malloc malloc malloc */
	timer.start();

	for(int i = 0; i < NUM_ITERS; i += NUM_MSG) {
		for(int j = 0; j < NUM_MSG; j++) {
			msg_arr[j] = (struct msg_t *) malloc(sizeof(struct msg_t));
			counter += (j + 1) * (int) (uintptr_t) msg_arr[j];
		}

		/* Freeing order does not matter much for malloc */
		for(int j = NUM_MSG - 1; j >= 0; j--) {
			free(msg_arr[j]);
		}
		
	}

	std::cout << "Malloc: " << timer.format() << "Counter: " << counter << "\n";
#endif

	/* Boost boost boost */
	timer.start();

	for(int i = 0; i < NUM_ITERS; i += NUM_MSG) {
		for(int j = 0; j < NUM_MSG; j++) {
			msg_arr[j] = pool.malloc();
			counter += (j + 1) * (int) (uintptr_t) msg_arr[j];
		}

		/* Freeing order matters a LOT for boost pools */
		for(int j = NUM_MSG - 1; j >= 0; j--) {
			pool.free(msg_arr[j]);
		}
	}

	std::cout << "Boost pool: " << timer.format() << "Counter: " << counter <<
		" Pool next size: " << pool.get_next_size() << "\n";
}
