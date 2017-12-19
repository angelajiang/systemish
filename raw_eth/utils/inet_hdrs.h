#ifndef INET_HDRS_H
#define INET_HDRS_H

#include <arpa/inet.h>
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <ifaddrs.h>
#include <linux/if_arp.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>

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

uint32_t ip_from_str(const char *ip) {
  uint32_t addr;
  int ret = inet_pton(AF_INET, ip, &addr);
  rt_assert(ret == 1, "inet_pton() failed");
  return addr;
}

std::string ip_to_string(uint32_t ipv4_addr) {
  static_assert(INET_ADDRSTRLEN == 16, "");
  char str[INET_ADDRSTRLEN];
  const char *ret = inet_ntop(AF_INET, &ipv4_addr, str, sizeof(str));
  rt_assert(ret == str, "inet_ntop failed");
  str[INET_ADDRSTRLEN - 1] = 0;  // Null-terminate
  return str;
}

static std::string get_interface_ipv4_str(std::string interface) {
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

static std::string get_interface_mac_str(std::string interface) {
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

static std::string get_ibdev_ipv4_string(std::string ibdev_name) {
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

#endif  // INET_HDRS_H
