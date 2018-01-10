/**
 * @file main.cc
 * @brief Test utils for raw Ethernet
 */

#include <stdio.h>
#include <thread>

int main() {
  unsigned num_hw_threads = std::thread::hardware_concurrency();
  printf("Hardware threads = %u\n", num_hw_threads);
}
