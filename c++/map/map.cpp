/** < Map pairs to structs */

#include <stdio.h>
#include <stdint.h>
#include <map>
using namespace std;

struct stats {
	long long tot_ref;
	long long tot_misses;
};

int main()
{
	std::map<std::pair<int, int>, struct stats> profile;

	struct stats st_1;
	st_1.tot_ref = 1;
	st_1.tot_misses = 3185;

	profile[std::make_pair(32, 0)] = st_1;

	struct stats out = profile[std::make_pair(32, 0)];
	printf("%lld %lld\n", out.tot_ref, out.tot_misses);

	out.tot_ref ++;
	profile[std::make_pair(32, 0)] = out;

	out = profile[std::make_pair(32, 0)];
	printf("%lld %lld\n", out.tot_ref, out.tot_misses);

	out = profile[std::make_pair(31, 0)];
	printf("%lld %lld\n", out.tot_ref, out.tot_misses);
}
