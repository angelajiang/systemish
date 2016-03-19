#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<assert.h>
#include <papi.h>

#define ITERS (32 * 1024 * 1024)

int main(int argc, char **argv)
{
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

	volatile long long *A = malloc(size);
	volatile long long *B = malloc(size);

	int i;
	for(i = 0; i < ITERS; i ++) {
		memcpy((void *) B, (void *) A, size);
		memcpy((void *) A, (void *) B, size);
	}

	retval = PAPI_ipc(&real_time, &proc_time, &ins, &ipc);
	if(retval < PAPI_OK) {    
		printf("PAPI error: retval: %d\n", retval);
		return -1;
	}

	printf("Time per memcpy = %f ns, instructions per memcpy = %lld, IPC = %.2f\n", 
		(real_time * 1000000000) / (ITERS * 2), ins / (ITERS * 2), ipc);

	return 0;
}
