#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

#define NUM_PKTS (256 * 1024 * 1024)
#define NUM_LONGS 10

int main(int argc, char **argv)
{
	printf("Starting computing hashes\n");
	struct timespec start, end;
	clock_gettime(CLOCK_REALTIME, &start);

	long long sum = 0;
	int i = 0, j = 0;
	long long A[NUM_LONGS] = {0};
	long long B[NUM_LONGS] = {0};

	for(i = 0; i < NUM_PKTS; i ++) {
		memcpy(B, A, NUM_LONGS * sizeof(long long));
		memcpy(A, B, NUM_LONGS * sizeof(long long));
	}

	// GCC is so clever, so print all
	for(j = 0; j < NUM_LONGS; j++) {
		printf("%lld\n", A[j]);
	}

	clock_gettime(CLOCK_REALTIME, &end);

	double seconds = (end.tv_sec - start.tv_sec) + 
		(double) (end.tv_nsec - start.tv_nsec) / 1000000000;
	double nanoseconds = (end.tv_sec - start.tv_sec) * 1000000000 + 
		(end.tv_nsec - start.tv_nsec);

	printf("Time = %f, time per memcpy = %f ns, sum = %lld\n", 
		seconds, nanoseconds / (NUM_PKTS * 2), sum);

	return 0;
}
