#include "udp_server.h"

static constexpr uint16_t global_udp_port = 3185;
static constexpr size_t max_msg_size = 1500;

int main() {
  UDPServer u(global_udp_port);
  char msg[max_msg_size];

  size_t num_rx = 0;
  while (true) {
    u.recv_blocking(msg, max_msg_size);
    printf("%zu: %s\n", num_rx++, msg);
  }
}
