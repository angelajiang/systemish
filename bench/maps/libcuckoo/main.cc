#include<stdio.h>
#include<stdlib.h>
#include<vector>
#include<time.h>
#include<assert.h>
#include<stdint.h>
#include<libcuckoo/cuckoohash_map.hh>
#include<chrono>
using namespace std;
using namespace std::chrono;

#define KEYS 3000000  /* 30 million */

static inline uint32_t hrd_fastrand(uint64_t *seed)
{
	*seed = *seed * 1103515245 + 12345;
	return (uint32_t)(*seed >> 32);
}

int main()
{
	long long sum = 1;
	printf("Starting warmup\n");
	// Warmup
	for(int i = 0; i < 1000000000; i++) {
		sum += sum * (i + sum);
	}
	printf("Warmup done\n");

	printf("Starting insertion\n");
	cuckoohash_map<uint64_t, uint64_t> M;
	for (int i = 0; i < KEYS; i++) {
		uint64_t key = i;
		uint64_t value = (key * key) + i;
		M[key] = value;
	}
	printf("Insertion done\n");

	uint64_t seed = 0xdeadbeef;
	uint64_t *A = new uint64_t[KEYS];
	for(int i = 0; i < KEYS; i++) {
		A[i] = hrd_fastrand(&seed);
	}

	std::chrono::high_resolution_clock timer;
	auto start = timer.now();
	uint64_t out;
	for (int i = 0; i < KEYS; i++) {
		if(M.find(A[i], out)) {
			sum += out;
		}
	}

	auto end = timer.now();
	double us = duration_cast<microseconds>(end - start).count();

	printf("Tput = %.2f M/s, sum = %lld\n", KEYS / us, sum);
}

