#include "udp_client.h"

static constexpr uint16_t global_udp_port = 3185;
static std::string dest_hostname = "fawn-pluto1";
static constexpr size_t msg_len = 100;
static constexpr size_t num_pkts = 1000;

int main() {
  UDPClient u(global_udp_port);
  char msg[msg_len];
  for (auto &c : msg) c = 'A';

  for (size_t i = 0; i < num_pkts; i++) {
    ssize_t ret = u.send(dest_hostname, msg, msg_len);
    if (ret != msg_len) throw std::runtime_error("send() failed");
  }
}
