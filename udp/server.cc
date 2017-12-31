#include "udp_server.h"

static constexpr uint16_t server_udp_port = 31850;
static constexpr size_t max_msg_size = 1500;
static constexpr size_t timeout_ms = 100;

int main() {
  UDPServer u(server_udp_port, timeout_ms, 1024 * 1024 * 4);
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
