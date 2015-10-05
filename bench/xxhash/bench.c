#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<time.h>
#include<assert.h>

#include "xxhash.h"

#define NUM_PKTS (1024 * 1024 * 1024)
#define NUM_LONGS 2

int main(int argc, char **argv)
{
	int seed = 0;
	while(1) {
		printf("Starting computing hashes for seed = %d\n", seed);
		struct timespec start, end;
		clock_gettime(CLOCK_REALTIME, &start);

		long long sum = 0;
		int i = 0, j = 0;
		long long A[NUM_LONGS];

		for(i = 0; i < NUM_PKTS; i ++) {
			for(j = 0; j < NUM_LONGS; j ++) {
				A[j] = 0xffffffffffffffffL + i + j;
			}
			sum += XXH64((char *) A, NUM_LONGS * sizeof(long long), 1);
		}

		clock_gettime(CLOCK_REALTIME, &end);
		double seconds = (end.tv_sec - start.tv_sec) + 
			(double) (end.tv_nsec - start.tv_nsec) / 1000000000;
		double nanoseconds = (end.tv_sec - start.tv_sec) * 1000000000 + 
			(end.tv_nsec - start.tv_nsec);

		printf("Time = %f, time per hash = %f ns, sum = %lld\n", 
			seconds, nanoseconds / NUM_PKTS, sum);

		seed++;
	}

	return 0;
}
