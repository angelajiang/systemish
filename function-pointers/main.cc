#include<stdio.h>
#include<stdlib.h>
#include<vector>
#include<time.h>
#include<assert.h>
#include<stdint.h>

#include "main.h"

/* An array of functions that take int arg and return long long */
long long (*rpc_func_arr[2])(int);

/* Register a function @func that takes int arg and returns long long */
void register_func(int index, long long (*func)(int))
{
	rpc_func_arr[index] = func;
}

int main()
{
	register_func(0, foo);
	register_func(1, bar);

	int x = 1;
	x = rpc_func_arr[0](x);
	x = rpc_func_arr[1](x);

	printf("x = %d\n", x);
}
