#include "foo.h"

template <int T>
Foo<T>::Foo() {
  printf("Foo constructor\n");
}

template <int T>
Foo<T>::~Foo() {
  printf("Foo destructor\n");
}

template <int T>
void Foo<T>::print_2(std::string x) {
  printf("print_2 %u: %s\n", T, x.c_str());
}
