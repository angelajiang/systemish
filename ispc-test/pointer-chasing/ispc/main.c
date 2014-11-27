#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<string.h>
#include<assert.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "common.h"

int *A;

void init_array()
{
	int i;

	printf("Initializing %lu bytes\n", CAP * sizeof(int));
	int shm_id = shmget(3185, CAP * sizeof(int), IPC_CREAT | 0666 | SHM_HUGETLB);
	if(shm_id == -1) {
		fprintf(stderr, "shmget Error! Failed to create array\n");
		exit(0);
	}

	A = (int *) shmat(shm_id, 0, 0);

	srand(41);
	for(i = 0; i < CAP; i++) {
		A[i] = rand() & CAP_;
	}
	
	printf("Done initializing\n");
}


/**< Non-ISPC version of pointer chasing */
void ptr_chase(const int *A, int *sum) {

	int i, j, addr[BATCH_SIZE];

	for(j = 0; j < BATCH_SIZE; j ++) {
		addr[j] = A[j];
	}

	for(i = 0; i < DEREF_LENGTH; i ++) {

		for(j = 0; j < BATCH_SIZE; j ++) {
			
			sum[j] += A[addr[j]];
			addr[j] = (A[addr[j]] + i) & CAP_;
		}
    }
}

int main(int argc, char **argv)
{
	int i;
	int sum[BATCH_SIZE];
	struct timespec start, end;

	assert(argc == 2);
	int use_ispc = atoi(argv[1]);
	
	memset(sum, 0, BATCH_SIZE * sizeof(int));

	/**< Initialize the log for pointer chasing */
	init_array();

	clock_gettime(CLOCK_REALTIME, &start);

	if(use_ispc == 1) {
		ispc_ptr_chase(A, sum);
	} else {
		ptr_chase(A, sum);
	}

	clock_gettime(CLOCK_REALTIME, &end);
	double seconds = (end.tv_sec - start.tv_sec) +
		(double) (end.tv_nsec - start.tv_nsec) / 1000000000;

	for(i = 0; i < BATCH_SIZE; i ++) {
		printf("sum[%d] = %d\n", i, sum[i]);
	}

	printf("Time = %f\n", seconds);
}
