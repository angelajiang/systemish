#include "udp_server.h"

static constexpr uint16_t kSrvUdpPort = 31850;
static constexpr size_t kTimeoutMs = 1000;

static constexpr size_t kMsgLen = 10;
static_assert(kMsgLen > 1, "");  // For null-termination
struct msg_t {
  char buf[kMsgLen];
};

int main() {
  erpc::UDPServer<msg_t> u(kSrvUdpPort, kTimeoutMs, 1024 * 1024 * 4);

  size_t num_rx = 0;
  while (true) {
    msg_t msg;
    int ret = u.recv_blocking(msg);
    if (ret < 0) {
      printf("%zu: No RX.\n", num_rx++);
    } else {
      printf("%zu: %s\n", num_rx++, msg.buf);
    }
  }
}
