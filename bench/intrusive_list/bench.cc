#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <boost/intrusive/list.hpp>
#include <vector>

static constexpr size_t kNumElements = 128;
static constexpr size_t kNumIters = 1000000;

typedef boost::intrusive::list_base_hook<
    boost::intrusive::link_mode<boost::intrusive::auto_unlink> >
    auto_unlink_hook;

class MyClass : public auto_unlink_hook {
  size_t x;

 public:
  MyClass(size_t x) : x(x) {}
  void unlink() { auto_unlink_hook::unlink(); }
};

typedef boost::intrusive::list<MyClass,
                               boost::intrusive::constant_time_size<false> >
    MemberList;

void test() {
  MemberList memberlist;
  struct timespec start, end;

  typedef std::vector<MyClass>::iterator VectIt;

  std::vector<MyClass> values;
  std::vector<MyClass *> rev_values;
  for (size_t i = 0; i < kNumElements; i++) values.push_back(MyClass(i));
  for (auto &v : values) rev_values.push_back(&v);

  clock_gettime(CLOCK_REALTIME, &start);

  for (size_t i = 0; i < kNumIters; i++) {
    for (auto &v : values) memberlist.push_back(v);
    for (auto *v : rev_values) v->unlink();
  }

  clock_gettime(CLOCK_REALTIME, &end);
  double tot_ns = (end.tv_sec - start.tv_sec) * 1000000000.0 +
                  (end.tv_nsec - start.tv_nsec);

  printf("RDTSC: Time per op = %.2f ns, size = %lu\n",
         tot_ns / (kNumIters * kNumElements * 2), memberlist.size());
}

int main() {
  for (size_t i = 0; i < 20; i++) test();
  return 0;
}
