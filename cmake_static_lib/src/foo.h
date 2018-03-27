#ifndef FOO_H
#define FOO_H

#include <stdio.h>
#include <string>

template <int T>
class Foo {
 public:
  Foo();
  ~Foo();

  void print_1(std::string x) { printf("print_1 %d: %s\n", T, x.c_str()); }

  void print_2(std::string x);
};

#endif
