#include<stdio.h>
#include<stdlib.h>
#include<vector>
#include<time.h>
#include<assert.h>
#include<stdint.h>
#include<unordered_map>
#include <boost/unordered_map.hpp>
#include<chrono>
using namespace std;
using namespace std::chrono;

#define KEYS 3000000  /* 30 million */

#define USE_BOOST 1

static inline uint32_t hrd_fastrand(uint64_t *seed)
{
	*seed = *seed * 1103515245 + 12345;
	return (uint32_t)(*seed >> 32);
}

int main()
{
	long long sum = 1;
	std::chrono::high_resolution_clock timer;

	printf("Starting warmup\n");
	// Warmup
	for(int i = 0; i < 1000000000; i++) {
		sum += sum * (i + sum);
	}
	printf("Warmup done\n");

	printf("Starting insertion\n");
	std::unordered_map<uint64_t, uint64_t> M;
	boost::unordered_map<uint64_t, uint64_t> Mboost;

	auto start = timer.now();
	for (int i = 0; i < KEYS; i++) {
		uint64_t key = i;
		uint64_t value = (key * key) + i;
#if USE_BOOST == 0
		M.emplace(key, value);
#else
		Mboost.emplace(key, value);
#endif
	}
	auto end = timer.now();

	double us = duration_cast<microseconds>(end - start).count();
	printf("Insert tput = %.2f M/s, sum = %lld\n", KEYS / us, sum);

	uint64_t seed = 0xdeadbeef;
	uint64_t *A = new uint64_t[KEYS];
	for(int i = 0; i < KEYS; i++) {
		A[i] = hrd_fastrand(&seed);
	}

	start = timer.now();
	for (int i = 0; i < KEYS; i++) {
#if USE_BOOST == 0
		auto result = M.find(A[i]);
		if(result != M.end()) {
			sum += result->second;
		}
#else
		auto result = Mboost.find(A[i]);
		if(result != Mboost.end()) {
			sum += result->second;
		}
#endif
	}

	end = timer.now();
	us = duration_cast<microseconds>(end - start).count();

	printf("Get = %.2f M/s, sum = %lld\n", KEYS / us, sum);
}
