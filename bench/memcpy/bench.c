#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<assert.h>
#include<malloc.h>
#include <papi.h>
#include "rte_memcpy.h"

#define ITERS (64 * 1024 * 1024)

#define TEST_DPDK_MEMCPY_CORRECTNESS 0
#define USE_GLIBC_ALIGN 0
#define USE_GLIBC_NOALIGN 0
#define USE_RTE_MEMCPY_NOALIGN 0
#define USE_RTE_MEMCPY_ALIGN 1

static inline uint32_t
rte_align32pow2(uint32_t x)
{
	x--;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;

	return x + 1;
}

void test_dpdk_memcpy_correctness()
{
	int i, j;
	for(i = 0; i < 1000; i++) {
		int size = rand() % 1000 + 1; 
		uint8_t *A = malloc(size);
		uint8_t *B = malloc(size);

		/* Fill random bytes into A */
		for(j = 0; j < size; j++) {
			A[j] = rand() % 256;
		}

		/* Set B to 0 */
		memset(B, 0, size);

		int lo = rand() % size;
		int hi = rand() % size;
		while(hi < lo) {
			hi = rand() % size;
		}
		int bytes_to_copy = hi - lo + 1;

		memcpy(&B[lo], &A[lo], bytes_to_copy);

		/* Check B */
		for(j = 0; j < size; j++) {
			if(j >= lo && j <= hi) {
				assert(B[j] == A[j]);
			} else {
				assert(B[j] == 0);
			}
		}

		if(i % 100 == 0) {
			printf("DPDK memcpy test passed up to test %d\n", i);
		}
	}

	printf("DPDK memcpy test passed\n");
}

int main(int argc, char **argv)
{
#if TEST_DPDK_MEMCPY_CORRECTNESS == 1
	test_dpdk_memcpy_correctness();
#endif

	int i;

	if(argc < 2) {
		printf("./bench <memcpy size>\n");
		exit(-1);
	}

	/* Variables for PAPI */
	float real_time, proc_time, ipc;
	long long ins;
	int retval;

	/* Init PAPI */
	if((retval = PAPI_ipc(&real_time, &proc_time, &ins, &ipc)) < PAPI_OK) {    
		printf("PAPI error: retval: %d\n", retval);
		return -1;
	}

	int size = atoi(argv[1]);

	volatile long long *A = memalign(64, rte_align32pow2(size));
	volatile long long *B = memalign(64, rte_align32pow2(size));

	for(i = 0; i < ITERS; i ++) {
#if USE_GLIBC_ALIGN == 1
		memcpy((void *) B, (void *) A, rte_align32pow2(size - 3));
		memcpy((void *) A, (void *) B, rte_align32pow2(size));
#elif USE_GLIBC_NOALIGN == 1
		memcpy((void *) B, (void *) A, size - 3);
		memcpy((void *) A, (void *) B, size);
#elif USE_RTE_MEMCPY_ALIGN == 1
		rte_memcpy((void *) B, (void *) A, rte_align32pow2(size - 3));
		rte_memcpy((void *) A, (void *) B, rte_align32pow2(size));
#elif USE_RTE_MEMCPY_NOALIGN == 1
		rte_memcpy((void *) B, (void *) A, size - 3);
		rte_memcpy((void *) A, (void *) B, size);
#endif
	}

	retval = PAPI_ipc(&real_time, &proc_time, &ins, &ipc);
	if(retval < PAPI_OK) {    
		printf("PAPI error: retval: %d\n", retval);
		return -1;
	}

	printf("%d \t %.2f \t %.2f\n", size, (real_time * 1000000000) / (ITERS * 2),
		(double) ins / (ITERS * 2));

	return 0;
}
