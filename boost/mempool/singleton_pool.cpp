#include <boost/pool/singleton_pool.hpp>
#include <boost/timer/timer.hpp>
#include <iostream>

#define NUM_ITERS 20000000
#define SIZE 65	/* Size of the struct */
#define NUM_MSG 64

#define USE_MALLOC 1

struct msg_t {
	char A[SIZE];
};

struct msg_pool {};
typedef boost::singleton_pool<msg_pool, sizeof(int)> singleton_msg_pool;

int main()
{
	struct msg_t *msg_arr[NUM_MSG];
	boost::timer::cpu_timer timer;
	int counter = 0;

#if USE_MALLOC == 1
	/* Malloc */
	timer.start();

	for(int i = 0; i < NUM_ITERS; i += NUM_MSG) {
		for(int j = 0; j < NUM_MSG; j++) {
			msg_arr[j] = static_cast<struct msg_t *>(singleton_msg_pool::malloc());
			counter += (j + 1) * (int) (uintptr_t) msg_arr[j];
		}

		singleton_msg_pool::purge_memory();
	}

	std::cout << "Boost: " << timer.format() << "Counter: " << counter << "\n";
#endif

	/* Boost */
	timer.start();

	for(int i = 0; i < NUM_ITERS; i += NUM_MSG) {
		for(int j = 0; j < NUM_MSG; j++) {
			msg_arr[j] = (struct msg_t *) malloc(sizeof(struct msg_t));
			counter += (j + 1) * (int) (uintptr_t) msg_arr[j];
		}

		for(int j = 0; j < NUM_MSG; j++) {
			free(msg_arr[j]);
		}
	}

	std::cout << "Malloc: " << timer.format() << "Counter: " << counter << "\n";
}
