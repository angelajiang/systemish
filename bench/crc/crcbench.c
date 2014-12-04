#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<time.h>
#include<assert.h>
#include<stdint.h>

#define NUM_PKTS (32 * 1024 * 1024)
#define NUM_LONGS 1

uint32_t fastcrc(char *str, uint32_t len) {
	uint32_t q = len / sizeof(uint32_t);
	uint32_t r = len % sizeof(uint32_t);
	uint32_t *p = (uint32_t*) str;
	uint32_t crc = 0;

	while(q --) {
		__asm__ __volatile__(
			".byte 0xf2, 0xf, 0x38, 0xf1, 0xf1;"
			:"=S" (crc)
			:"0" (crc), "c" (*p)
		);
		p++;
	}

	str = (char*) p;
	while(r --) {
		__asm__ __volatile__(
			".byte 0xf2, 0xf, 0x38, 0xf0, 0xf1"
			:"=S" (crc)
			:"0" (crc), "c" (*str)
		);
		str++;
	}

	return crc;
}

int main(int argc, char **argv)
{
	printf("Starting computing crcs\n");
	struct timespec start, end;
	clock_gettime(CLOCK_REALTIME, &start);

	long long sum = 0;
	int i = 0, j = 0;
	long long A[NUM_LONGS];

	for(i = 0; i < NUM_PKTS; i ++) {
		for(j = 0; j < NUM_LONGS; j ++) {
			A[j] = 0xffffffffffffffffL + i;
		}
		int crc = fastcrc((char *) A, NUM_LONGS * sizeof(long long));
		sum += crc;
	}

	clock_gettime(CLOCK_REALTIME, &end);
	double seconds = (end.tv_sec - start.tv_sec) + 
		(double) (end.tv_nsec - start.tv_nsec) / 1000000000;
	double nanoseconds = (end.tv_sec - start.tv_sec) * 1000000000 + 
		(end.tv_nsec - start.tv_nsec);

	printf("Time = %f, time per crc = %f ns, sum = %lld\n", 
		seconds, nanoseconds / NUM_PKTS, sum);

	return 0;
}
