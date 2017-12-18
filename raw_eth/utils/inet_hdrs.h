#ifndef INET_HDRS_H
#define INET_HDRS_H

#include <arpa/inet.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <sstream>
#include <string>

#define _unused(x) ((void)(x))  // Make production build happy
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

/// Check a condition at runtime. If the condition is false, throw exception.
static inline void rt_assert(bool condition, std::string throw_str) {
  if (unlikely(!condition)) throw std::runtime_error(throw_str);
}

/// Check a condition at runtime. If the condition is false, throw exception.
static inline void rt_assert(bool condition) {
  if (unlikely(!condition)) throw std::runtime_error("Error");
}

std::string mac_to_string(const uint8_t mac[6]) {
  std::ostringstream ret;
  for (size_t i = 0; i < 6; i++) {
    ret << std::hex << static_cast<uint32_t>(mac[i]);
    if (i != 5) ret << ":";
  }
  return ret.str();
}

uint32_t ip_from_str(const char* ip) {
  uint32_t addr;
  int ret = inet_pton(AF_INET, ip, &addr);
  rt_assert(ret == 1, "inet_pton() failed");
  return addr;
}

std::string ip_to_string(uint32_t ipv4_addr) {
  static_assert(INET_ADDRSTRLEN == 16, "");
  char str[INET_ADDRSTRLEN];
  const char* ret = inet_ntop(AF_INET, &ipv4_addr, str, sizeof(str));
  rt_assert(ret == str, "inet_ntop failed");
  str[INET_ADDRSTRLEN - 1] = 0;  // Null-terminate
  return str;
}

#endif  // INET_HDRS_H
