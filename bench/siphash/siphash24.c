/* This code is based on, and distributed under the same license as:

 SipHash reference C implementation

 Written in 2012 by
 Jean-Philippe Aumasson <jeanphilippe.aumasson@gmail.com>
 Daniel J. Bernstein <djb@cr.yp.to>

 To the extent possible under law, the author(s) have dedicated all copyright
 and related and neighboring rights to this software to the public domain
 worldwide. This software is distributed without any warranty.

 You should have received a copy of the CC0 Public Domain Dedication along with
 this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.

 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define ROTL(x,b) (uint64_t)(((x) << (b)) | ((x) >> (64 - (b))))

#define U8TO64_LE(p) \
(((uint64_t)((p)[0])      ) | \
((uint64_t)((p)[1]) <<  8) | \
((uint64_t)((p)[2]) << 16) | \
((uint64_t)((p)[3]) << 24) | \
((uint64_t)((p)[4]) << 32) | \
((uint64_t)((p)[5]) << 40) | \
((uint64_t)((p)[6]) << 48) | \
((uint64_t)((p)[7]) << 56))

#define SIPROUND            \
do {              \
v0 += v1; v1=ROTL(v1,13); v1 ^= v0; v0=ROTL(v0,32); \
v2 += v3; v3=ROTL(v3,16); v3 ^= v2;     \
v0 += v3; v3=ROTL(v3,21); v3 ^= v0;     \
v2 += v1; v1=ROTL(v1,17); v1 ^= v2; v2=ROTL(v2,32); \
} while(0)

/* SipHash-2-4 */
uint64_t
siphash_2_4(const unsigned char *in, size_t inlen, const unsigned char *k)
{
	/* "somepseudorandomlygeneratedbytes" */
	uint64_t v0 = 0x736f6d6570736575ULL;
	uint64_t v1 = 0x646f72616e646f6dULL;
	uint64_t v2 = 0x6c7967656e657261ULL;
	uint64_t v3 = 0x7465646279746573ULL;
	uint64_t b;
	uint64_t k0 = U8TO64_LE(k);
	uint64_t k1 = U8TO64_LE(k + 8);
	uint64_t m;
	const uint8_t *end = in + inlen - (inlen % sizeof(uint64_t));
	const int left = inlen & 7;
	b = ((uint64_t)inlen) << 56;
	v3 ^= k1;
	v2 ^= k0;
	v1 ^= k1;
	v0 ^= k0;

	for (; in != end; in += 8) {
		m = U8TO64_LE(in);
		v3 ^= m;
		SIPROUND;
		SIPROUND;
		v0 ^= m;
	}

	switch (left) {
		case 7:
			b |= ((uint64_t)in[6])  << 48;
	
		case 6:
			b |= ((uint64_t)in[5])  << 40;
	
		case 5:
			b |= ((uint64_t)in[4])  << 32;
	
		case 4:
			b |= ((uint64_t)in[3])  << 24;
	
		case 3:
			b |= ((uint64_t)in[2])  << 16;
	
		case 2:
			b |= ((uint64_t)in[1])  <<  8;
	
		case 1:
			b |= ((uint64_t)in[0]);
			break;
	
		case 0:
			break;
	}

	v3 ^= b;
	SIPROUND;
	SIPROUND;
	v0 ^= b;
	v2 ^= 0xff;
	SIPROUND;
	SIPROUND;
	SIPROUND;
	SIPROUND;
	b = v0 ^ v1 ^ v2  ^ v3;
	return b;
}

#define NUM_PKTS (16 * 1024 * 1024)
#define NUM_LONGS 16

static inline long long hrd_get_cycles()
{
	unsigned low, high;
	unsigned long long val;
	asm volatile ("rdtsc" : "=a" (low), "=d" (high));
	val = high;
	val = (val << 32) | low;
	return val;
}

int main()
{
	printf("Starting computing hashes\n");
	struct timespec start, end;
	clock_gettime(CLOCK_REALTIME, &start);

	long long sum = 0;
	int i = 0, j = 0, key = 25;
	long long A[NUM_LONGS];

	for(i = 0; i < NUM_PKTS; i ++) {
		// Initialize the array to compute the hash off
		for(j = 0; j < NUM_LONGS; j ++) {
			A[j] = 0xffffffffffffffffL + i;
		}

		// Do useless computation
		int useless = 0;
		for(useless = 0; useless < 1000000000; useless++) {
			sum += (sum + i + useless) * (sum + i + useless);
		}

		// Compute the hash
		long long start_cycles = hrd_get_cycles() + (sum & 1);
		sum += siphash_2_4((char *) A, 
			NUM_LONGS * sizeof(long long),
			(char *) &key);		/**< siphash is a keyed hash function */

		long long end_cycles = hrd_get_cycles() + (sum & 1);
		printf("%lld\n", end_cycles - start_cycles);
	}

	clock_gettime(CLOCK_REALTIME, &end);
	double seconds = (end.tv_sec - start.tv_sec) + 
		(double) (end.tv_nsec - start.tv_nsec) / 1000000000;
	double nanoseconds = (end.tv_sec - start.tv_sec) * 1000000000 + 
		(end.tv_nsec - start.tv_nsec);

	printf("Time = %f, time per hash = %f ns, sum = %lld\n", 
		seconds, nanoseconds / NUM_PKTS, sum);

	return 0;
}
