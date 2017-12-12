#include "common.h"
#include "rand.h"

void format_packet(uint8_t *buf) {
  gen_eth_header(reinterpret_cast<eth_hdr_t *>(buf), kSrcMAC, kDstMAC,
                 kIPEtherType);

  buf += sizeof(eth_hdr_t);
  uint32_t src_ip = ip_from_str(kSrcIP);
  uint32_t dst_ip = ip_from_str(kDstIP);
  gen_ipv4_header(reinterpret_cast<ipv4_hdr_t *>(buf), src_ip, dst_ip,
                  kIPHdrProtocol, kDataSize);

  buf += sizeof(ipv4_hdr_t);
  uint16_t src_port = 0xffffu;  // It doesn't matter what your source port is!
  gen_udp_header(reinterpret_cast<udp_hdr_t *>(buf), src_port, kBaseDstPort,
                 kDataSize);
}

int main() {
  ctrl_blk_t *cb = init_ctx(kDeviceIndex);
  FastRand fast_rand;

  static constexpr size_t pkt_sz =
      sizeof(eth_hdr_t) + sizeof(ipv4_hdr_t) + sizeof(udp_hdr_t) + kDataSize;

  uint8_t packet[pkt_sz] = {0};
  format_packet(packet);

  for (auto u : packet) printf("%02x ", u);
  printf("\n");

  struct ibv_sge sg_entry;
  sg_entry.addr = reinterpret_cast<uint64_t>(packet);
  sg_entry.length = pkt_sz;
  sg_entry.lkey = 0;

  struct ibv_send_wr wr;
  memset(&wr, 0, sizeof(wr));
  wr.num_sge = 1;
  wr.sg_list = &sg_entry;
  wr.next = nullptr;
  wr.opcode = IBV_WR_SEND;

  // Do SENDS
  size_t nb_tx = 0;
  while (true) {
    wr.send_flags = IBV_SEND_INLINE;
    wr.wr_id = static_cast<size_t>(nb_tx);
    wr.send_flags |= IBV_SEND_SIGNALED;

    // Direct the packet to one of the receiver threads
    udp_hdr_t *udp_hdr = reinterpret_cast<udp_hdr_t *>(
        packet + sizeof(eth_hdr_t) + sizeof(ipv4_hdr_t));
    udp_hdr->dst_port =
        htons(kBaseDstPort + fast_rand.next_u32() % kReceiverThreads);

    struct ibv_send_wr *bad_wr;
    int ret = ibv_post_send(cb->qp, &wr, &bad_wr);
    usleep(20000);
    if (ret < 0) {
      fprintf(stderr, "Failed in post send\n");
      exit(1);
    }

    if (nb_tx++ % 1000000 == 0) printf("Sent %zu packets\n", nb_tx);

    struct ibv_wc wc;
    ret = ibv_poll_cq(cb->send_cq, 1, &wc);
    assert(ret >= 0);
  }
  return 0;
}
