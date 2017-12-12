#include <arpa/inet.h>
#include <assert.h>
#include <infiniband/verbs.h>
#include <infiniband/verbs_exp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static constexpr size_t kPortIndex = 2;       // mlx5_0
static constexpr size_t kPktSize = 60;        // Packet size, including headers
static constexpr size_t kRecvBufSize = 1500;  // RECV buffer size

uint8_t kDstMAC[6] = {0xec, 0x0d, 0x9a, 0x7b, 0xd7, 0xd6};
uint8_t kDstIP[4] = {192, 168, 1, 250};

uint8_t kSrcMAC[6] = {0xec, 0x0d, 0x9a, 0x7b, 0xd7, 0xe6};
uint8_t kSrcIP[4] = {192, 168, 1, 251};

struct eth_hdr_t {
  uint8_t dst_mac[6];
  uint8_t src_mac[6];
  uint16_t eth_type;
} __attribute__((packed));

struct ipv4_hdr_t {
  uint8_t ihl : 4;
  uint8_t version : 4;
  uint8_t tos;
  uint16_t tot_len;
  uint16_t id;
  uint16_t frag_off;
  uint8_t ttl;
  uint8_t protocol;
  uint16_t check;
  uint32_t saddr;
  uint32_t daddr;
} __attribute__((packed));

int ip_from_str(char *ip, uint32_t *addr) {
  return inet_pton(AF_INET, ip, addr);
}

static uint16_t ip_checksum(void *buf, size_t hdr_len) {
  unsigned long sum = 0;
  const uint16_t *ip1 = reinterpret_cast<uint16_t *>(buf);
  while (hdr_len > 1) {
    sum += *ip1++;
    if (sum & 0x80000000) sum = (sum & 0xFFFF) + (sum >> 16);
    hdr_len -= 2;
  }
  while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);

  return (~sum);
}

void gen_ipv4_header(void *ip_header_buffer, uint32_t *saddr, uint32_t *daddr,
                     uint8_t protocol, int pkt_size) {
  ipv4_hdr_t ip_header;
  memset(&ip_header, 0, sizeof(ip_header));

  ip_header.version = 4;
  ip_header.ihl = 5;
  ip_header.tos = 0;
  ip_header.tot_len = htons(pkt_size);
  ip_header.id = htons(0);
  ip_header.frag_off = htons(0);
  ip_header.ttl = 128;
  ip_header.protocol = protocol;
  ip_header.saddr = *saddr;
  ip_header.daddr = *daddr;
  ip_header.check = ip_checksum(&ip_header, sizeof(ip_header));
  memcpy(ip_header_buffer, &ip_header, sizeof(ip_header));
}

void gen_eth_header(eth_hdr_t *eth_header, uint8_t *src_mac, uint8_t *dst_mac,
                    uint16_t eth_type) {
  memcpy(eth_header->src_mac, src_mac, 6);
  memcpy(eth_header->dst_mac, dst_mac, 6);
  eth_header->eth_type = htons(eth_type);
}
