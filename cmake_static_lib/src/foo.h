#ifndef FOO_H
#define FOO_H

#include <stdio.h>
#include <string>

class Foo {
 public:
  Foo();
  ~Foo();

  void print_1(std::string x) {
    printf("print_1: %s\n", x.c_str());
  }

  void print_2(std::string x);
};

#endif
