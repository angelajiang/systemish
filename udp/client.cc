#include <assert.h>
#include "udp_client.h"

static std::string dest_hostname = "192.168.18.2";
static constexpr uint16_t kDstPort = 41851;
static constexpr size_t kMsgLen = 10;
static_assert(kMsgLen > 1, "");  // For null-termination
static constexpr size_t kNumPkts = 1000;

struct msg_t {
  char buf[kMsgLen];
};

int main() {
  erpc::UDPClient<msg_t> u;

  msg_t msg;
  for (auto &c : msg.buf) c = 'A';
  msg.buf[kMsgLen - 1] = 0;

  for (size_t i = 0; i < kNumPkts; i++) {
    ssize_t ret = u.send(dest_hostname, kDstPort, msg);
    assert(ret == static_cast<ssize_t>(kMsgLen));
  }
}
