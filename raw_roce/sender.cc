#include "common.h"

int main() {
  ctrl_blk_t *cb = init_ctx(kDeviceIndex);

  uint8_t packet[kPktSize] = {0};  // Minimum-sized Ethernet frame
  gen_eth_header(reinterpret_cast<eth_hdr_t *>(packet), kSrcMAC, kDstMAC,
                 static_cast<uint16_t>(kPktSize - sizeof(eth_hdr_t)));
  for (auto u : packet) printf("%02x ", u);
  printf("\n");

  struct ibv_sge sg_entry;
  sg_entry.addr = reinterpret_cast<uint64_t>(packet);
  sg_entry.length = sizeof(packet);
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
