#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<time.h>
#include<string.h>
#include<assert.h>

#define NUM_PKTS (128 * 1024 * 1024)
#define NUM_CHARS 16
#define NUM_CHARS_ 15

int main(int argc, char **argv)
{
	printf("Starting strlenbench\n");
	struct timespec start, end;

	long long sum = 0;
	int i = 0, j = 0;
	char A[NUM_CHARS];

	for(j = 0; j < NUM_CHARS; j ++) {
		A[j] = 1;
	}

	A[NUM_CHARS - 1] = 0;

	clock_gettime(CLOCK_REALTIME, &start);

	for(i = 0; i < NUM_PKTS; i ++) {
		int index = (i & NUM_CHARS_);
		A[index] = 0;
		sum += strlen(A);
		A[index] = 1;
	}

	clock_gettime(CLOCK_REALTIME, &end);
	double seconds = (end.tv_sec - start.tv_sec) + 
		(double) (end.tv_nsec - start.tv_nsec) / 1000000000;
	double nanoseconds = (end.tv_sec - start.tv_sec) * 1000000000 + 
		(end.tv_nsec - start.tv_nsec);

	printf("Time = %f, time per strlen = %f ns, sum = %lld\n", 
		seconds, nanoseconds / NUM_PKTS, sum);

	return 0;
}
