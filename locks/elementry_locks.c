/*
 * Two threads which increment a counter.
 * Observations:
 * 1. If both run in parallel without locks, the final answer is much less
 * than 2*MAX. It's more like 1.1*MAX. This is kind of expected because of the
 * time gap between a memory load and the corresponding writes to a register.
 *
 * 2. With fine grained locking: if both are run parallely with, the time required
 * is much more than that when run sequentially. This is due to bus banging.
 **/
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

#define USE_SPINLOCK 1

void *counter_incrementer_function(void *ptr);

/* A counter and a mutex to protect it */
int counter = 0;
int MAX = 5000000;

pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_spinlock_t counter_spinlock;

main()
{
	pthread_spin_init(&counter_spinlock, 0);
	pthread_t thread1, thread2, thread3, thread4;

	pthread_create(&thread1, NULL, counter_incrementer_function, NULL);
	pthread_create(&thread2, NULL, counter_incrementer_function, NULL);
	pthread_create(&thread3, NULL, counter_incrementer_function, NULL);
	pthread_create(&thread4, NULL, counter_incrementer_function, NULL);

	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
	pthread_join(thread3, NULL);
	pthread_join(thread4, NULL);

	printf("Counter = %d \n", counter);
	exit(0);
}

void *counter_incrementer_function( void *ptr)
{
	int i;
#if USE_SPINLOCK == 0
	for(i = 0; i < MAX; i++) {
		pthread_mutex_lock(&counter_mutex);
		counter++;
		if(counter % 1000000 == 0) {
			printf("%d\n", counter);
		}
		pthread_mutex_unlock(&counter_mutex);
	}
#else 
	for(i = 0; i < MAX; i++) {
		pthread_spin_lock(&counter_spinlock);
		counter++;
		if(counter % 1000000 == 0) {
			printf("%d\n", counter);
		}
		pthread_spin_unlock(&counter_spinlock);
	}
#endif
}
