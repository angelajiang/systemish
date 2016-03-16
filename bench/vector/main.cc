#include<stdio.h>
#include<stdlib.h>
#include<vector>
#include<time.h>
#include<assert.h>

#include "trivial_vector.h"

#define ITERS 10000000	/* 10 million */
#define SIZE 16

/*
 * Time per push_back:
 * 1. E5-2683-v3 at 3 GHz: std::vector: 1.99 ns, trivial_vector: .2 ns
 */
using namespace std;

int main()
{
	int iter;
	struct timespec start, end;
	double nanoseconds;

	// Trivial vector
	trivial_vector<int> tV;
	tV.init(SIZE);
	clock_gettime(CLOCK_REALTIME, &start);
	
	for(iter = 0; iter < 100000000; iter++) {
		for(int i = 0; i < SIZE; i++) {
			tV.push_back(i);
		}
		tV.clear();
	}

	clock_gettime(CLOCK_REALTIME, &end);

	nanoseconds = (end.tv_sec - start.tv_sec) * 1000000000 + 
		(end.tv_nsec - start.tv_nsec);

	printf("Time per push_back = %f ns\n", nanoseconds / (iter * SIZE));

	// Std vector
	std::vector<int> V;
	V.reserve(SIZE);

	clock_gettime(CLOCK_REALTIME, &start);
	for(iter = 0; iter < ITERS; iter++) {
		for(int i = 0; i < SIZE; i++) {
			V.push_back(i);
		}
		V.clear();
	}
	clock_gettime(CLOCK_REALTIME, &end);

	nanoseconds = (end.tv_sec - start.tv_sec) * 1000000000 + 
		(end.tv_nsec - start.tv_nsec);

	printf("Time per push_back = %f ns\n", nanoseconds / (iter * SIZE));

}
