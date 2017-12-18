/**
 * @file main.cc
 * @brief Test utils for raw Ethernet
 */

#include <assert.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "inet_hdrs.h"

void test_ip_to_str() {
  // 1.2.3.4
  uint32_t ip_addr = (4u << 24) | (3u << 16) | (2u << 8) | (1u << 0);
  printf("ip address to string = %s\n", ip_to_string(ip_addr).c_str());
}

void test_str_to_ip() {
  const char ip_str[] = "1.2.3.4";
  printf("ip address from string = %x\n", ip_from_str(ip_str));

  // Test if we can convert back and forth
  const char ip_str2[] = "192.168.1.255";
  uint32_t ip_addr2 = ip_from_str(ip_str2);
  printf("ip address from string = %x\n", ip_addr2);

  assert(strcmp(ip_str2, ip_to_string(ip_addr2).c_str()) == 0);
}

void test_mac_to_string() {
  const uint8_t mac[6] = {0xfe, 0xd5, 0xaa, 0x32, 0x11, 0xff};
  printf("mac address to string = %s\n", mac_to_string(mac).c_str());
}

std::string interface_to_ip_str(std::string interface) {
  struct ifaddrs *ifaddr, *ifa;

  int ret = getifaddrs(&ifaddr);
  assert(ret == 0);

  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr->sa_family != AF_INET) continue;

    if (strcmp(ifa->ifa_name, interface.c_str()) == 0) {
      auto sin_addr = reinterpret_cast<sockaddr_in *>(ifa->ifa_addr);
      uint32_t ipv4_addr = *reinterpret_cast<uint32_t *>(&sin_addr->sin_addr);
      std::string ip_str = ip_to_string(ipv4_addr);

      freeifaddrs(ifaddr);
      return ip_str;
    }
  }

  freeifaddrs(ifaddr);
  return "";
}

int main() {
  test_ip_to_str();
  test_str_to_ip();
  test_mac_to_string();

  printf("\n");

  std::string iface_arr[] = {"enp4s0f0", "enp4s0f1", "enp132s0f0", "enp4s0f1"};
  for (auto s : iface_arr) {
    std::string ip_str = interface_to_ip_str(s);
    printf("Interface %s, IP %s\n", s.c_str(),
           ip_str.length() != 0 ? ip_str.c_str() : "N/A");
  }
}
