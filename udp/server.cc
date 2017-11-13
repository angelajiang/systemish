#include "udp_server.h"

static constexpr uint16_t global_udp_port = 3185;
static constexpr size_t max_msg_size = 1500;
static constexpr size_t timeout_ms = 100;

int main() {
  UDPServer u(global_udp_port, timeout_ms);
  char msg[max_msg_size];

  size_t num_rx = 0;
  while (true) {
    int ret = u.recv_blocking(msg, max_msg_size);
    if (ret < 0) {
      printf("%zu: No RX.\n", num_rx++);
    } else {
      printf("%zu: %s\n", num_rx++, msg);
    }
  }
}
