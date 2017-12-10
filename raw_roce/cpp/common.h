#include <stdint.h>
#include <stdlib.h>

static constexpr size_t kPortIndex = 2;  // mlx5_0
static constexpr size_t kPktSize = 60;   // Packet size, including headers
uint8_t kDstMAC[6] = {0xec, 0x0d, 0x9a, 0x7b, 0xd7, 0xd6};
uint8_t kSrcMAC[6] = {0xec, 0x0d, 0x9a, 0x7b, 0xd7, 0xe6};
