#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
using namespace std;

class udp_client {
 public:
  udp_client(const char *addr, int port) : port(port), f_addr(addr) {
    char decimal_port[16];
    snprintf(decimal_port, sizeof(decimal_port), "%d", port);
    decimal_port[sizeof(decimal_port) / sizeof(decimal_port[0]) - 1] = '\0';

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    int r = getaddrinfo(addr, decimal_port, &hints, &f_addrinfo);
    if (r != 0 || f_addrinfo == NULL) {
      printf("udp_client: Invalid address or port\n");
      exit(-1);
    }

    sock_fd = socket(f_addrinfo->ai_family, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_fd == -1) {
      freeaddrinfo(f_addrinfo);
      printf("udp_client: Could not create socket\n");
      exit(-1);
    }
  }

  ~udp_client() {
    freeaddrinfo(f_addrinfo);
    close(sock_fd);
  }

  ssize_t send(const char *msg, size_t size) {
    return sendto(sock_fd, msg, size, 0, f_addrinfo->ai_addr,
                  f_addrinfo->ai_addrlen);
  }

 private:
  int sock_fd;
  int port;
  std::string f_addr;
  struct addrinfo *f_addrinfo;
};

int main() {
  udp_client c("128.2.211.34", 3490);

  for (int i = 0; i < 100000; i++) {
    char buf[100];
    sprintf(buf, "packet-%d", i);
    c.send(buf, strlen(buf));
    usleep(200000);
  }
}
