/**
 * @file main.cc
 * @brief Test utils for raw Ethernet
 */

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

int main() {
  test_ip_to_str();
  test_str_to_ip();
  test_mac_to_string();

  printf("\n");

  std::string net_iface_arr[] = {"enp4s0f0", "enp4s0f1", "enp132s0f0",
                                 "enp132s0f1"};
  for (auto ni : net_iface_arr) {
    std::string ip_str = get_interface_ipv4_str(ni);
    printf("Interface %s, IP %s\n", ni.c_str(),
           ip_str.length() != 0 ? ip_str.c_str() : "N/A");

    std::string mac_str = get_interface_mac_str(ni);
    printf("Interface %s, MAC %s\n", ni.c_str(),
           mac_str.length() != 0 ? mac_str.c_str() : "N/A");
  }

  printf("\n");

  std::string ibdev_arr[] = {"mlx5_0", "mlx5_1", "mlx5_2", "mlx5_3"};
  for (auto s : ibdev_arr) {
    std::string ip_str = get_ibdev_ipv4_string(s);
    printf("IB device %s, IP %s\n", s.c_str(),
           ip_str.length() != 0 ? ip_str.c_str() : "N/A");
  }
}
