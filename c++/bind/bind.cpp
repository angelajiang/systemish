#include <functional>
#include <iostream>
#include <assert.h>

using namespace std::placeholders;

class A {
 public:
  std::function< void(int, int) > bar;

  void foo_bound(int x, int y) {
    bar(x, y);
  }
};

void foo(int n1, int n2, int n3, int n4) {
  std::cout << n1 << " " << n2 <<  " " << n3 << " " << n4 << " " << std::endl;
}

int main(int argc, char **argv) {
  assert(argc == 2);
  int n = atoi(argv[1]);
  A a;

  a.bar = std::bind(foo, _1, n, n * 2, _2);
  a.foo_bound(n, n * 2);
}
