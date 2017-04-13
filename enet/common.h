#include <stdint.h>
#include <stdlib.h>

static constexpr uint16_t kUdpPort = 12345;
static constexpr size_t kMaxConnections = 4095;  // Max supported by enet
static constexpr size_t kChannels = 1;
static constexpr size_t kDataSize = 1000000;  // 10 MB

/// The void* data associated with an ENET peer
class PeerData {
 public:
  uint32_t id;
  PeerData(uint32_t id) : id(id) {}
};

