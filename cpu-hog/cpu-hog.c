#include<stdio.h>
#include<stdlib.h>

#define ITERS (1024 * 1024 * 1024)
int main()
{
	int i, sum = 23;
	for(i = 0; i < ITERS; i++) {
		sum += i;
	}
	printf("sum = %d\n", sum);
}
