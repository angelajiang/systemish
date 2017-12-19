/**
 * @file main.cc
 * @brief Test utils for raw Ethernet
 */

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <ifaddrs.h>
#include <linux/if_arp.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
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

std::string get_interface_ipv4_str(std::string interface) {
  struct ifaddrs *ifaddr, *ifa;

  int ret = getifaddrs(&ifaddr);
  assert(ret == 0);

  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr->sa_family != AF_INET) continue;  // IP address
    if (strcmp(ifa->ifa_name, interface.c_str()) != 0) continue;

    auto sin_addr = reinterpret_cast<sockaddr_in *>(ifa->ifa_addr);
    uint32_t ipv4_addr = *reinterpret_cast<uint32_t *>(&sin_addr->sin_addr);
    std::string ip_str = ip_to_string(ipv4_addr);

    freeifaddrs(ifaddr);
    return ip_str;
  }

  freeifaddrs(ifaddr);
  return "";
}

std::string get_interface_mac_str(std::string interface) {
  struct ifreq ifr;
  ifr.ifr_addr.sa_family = AF_INET;
  strncpy(ifr.ifr_name, interface.c_str(), IFNAMSIZ - 1);

  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  assert(fd >= 0);
  int ret = ioctl(fd, SIOCGIFHWADDR, &ifr);
  rt_assert(ret == 0, "MAC address IOCTL failed");

  close(fd);
  return mac_to_string(reinterpret_cast<uint8_t *>(ifr.ifr_hwaddr.sa_data));
}

std::string get_ibdev_ipv4_addr(std::string ibdev_name) {
  std::string dev_dir = "/sys/class/infiniband/" + ibdev_name + "/device/net";

  std::vector<std::string> net_ifaces;
  DIR *dp;
  struct dirent *dirp;
  dp = opendir(dev_dir.c_str());
  assert(dp != nullptr);

  while (true) {
    dirp = readdir(dp);
    if (dirp == nullptr) break;

    if (strcmp(dirp->d_name, ".") == 0) continue;
    if (strcmp(dirp->d_name, "..") == 0) continue;
    net_ifaces.push_back(std::string(dirp->d_name));
  }
  closedir(dp);

  assert(net_ifaces.size() > 0);
  return get_interface_ipv4_str(net_ifaces[0]);
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
    std::string ip_str = get_ibdev_ipv4_addr(s);
    printf("IB device %s, IP %s\n", s.c_str(),
           ip_str.length() != 0 ? ip_str.c_str() : "N/A");
  }
}
