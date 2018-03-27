#include "foo.h"

Foo::Foo() {
  printf("Foo constructor\n");
}

Foo::~Foo() {
  printf("Foo destructor\n");
}

void Foo::print_2(std::string x) {
  printf("print_2: %s\n", x.c_str());
}
