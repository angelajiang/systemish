#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <map>
#include <stdexcept>
#include <string>

/// Basic UDP server class that supports receiving messages
class UDPServer {
 public:
  UDPServer(uint16_t global_udp_port) : global_udp_port(global_udp_port) {
    sock_fd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_UDP);
    if (sock_fd == -1) {
      throw std::runtime_error("UDPServer: Failed to create local socket.");
    }

    char _port_str[16];
    snprintf(_port_str, sizeof(_port_str), "%u", global_udp_port);

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    struct addrinfo *local_addrinfo;
    int r = getaddrinfo("localhost", _port_str, &hints, &local_addrinfo);
    if (r != 0 || local_addrinfo == nullptr) {
      throw std::runtime_error("UDPServer: Failed to resolve localhost.");
    }

    if (local_addrinfo->ai_family != AF_INET) {
      throw std::runtime_error("UDPServer: ai_family mismatc.");
    }

    r = bind(sock_fd, local_addrinfo->ai_addr, local_addrinfo->ai_addrlen);
    if (r != 0) {
      throw std::runtime_error("UDPServer: Failed to bind socket.");
    }

    freeaddrinfo(local_addrinfo);
  }

  int recv_blocking(char *msg, size_t max_size) {
    return recv(sock_fd, msg, max_size, 0);
  }

 private:
  const uint16_t global_udp_port;
  int sock_fd;
};
