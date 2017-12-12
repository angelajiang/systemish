#include <assert.h>
#include <infiniband/verbs_exp.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

int main() {
  ctrl_blk_t *cb = init_ctx(kDeviceIndex);

  // Register RX ring memory
  size_t ring_size = kRecvBufSize * kRQDepth;
  void *buf = malloc(ring_size);
  assert(buf != nullptr);

  struct ibv_mr *mr =
      ibv_reg_mr(cb->pd, buf, ring_size, IBV_ACCESS_LOCAL_WRITE);
  assert(mr != nullptr);

  // Attach all buffers to the ring
  struct ibv_sge sge;
  sge.length = kRecvBufSize;
  sge.lkey = mr->lkey;

  struct ibv_recv_wr wr, *bad_wr;
  wr.num_sge = 1;
  wr.sg_list = &sge;
  wr.next = nullptr;

  for (size_t n = 0; n < kRQDepth; n++) {
    sge.addr = reinterpret_cast<uint64_t>(buf) + (kRecvBufSize * n);
    wr.wr_id = n;
    int ret = ibv_post_recv(cb->qp, &wr, &bad_wr);
    assert(ret == 0);
  }

  static constexpr size_t rule_sz =
      sizeof(ibv_exp_flow_attr) + sizeof(ibv_exp_flow_spec_eth);
  static_assert(rule_sz == 64, "");

  uint8_t *flow_rule = new uint8_t[rule_sz];
  memset(flow_rule, 0, rule_sz);

  auto *flow_attr = reinterpret_cast<struct ibv_exp_flow_attr *>(flow_rule);
  flow_attr->type = IBV_EXP_FLOW_ATTR_NORMAL;
  flow_attr->size = 64;
  flow_attr->priority = 0;
  flow_attr->num_of_specs = 1;
  flow_attr->port = 1;
  flow_attr->flags = 0;
  flow_attr->reserved = 0;

  auto *flow_spec_eth = reinterpret_cast<struct ibv_exp_flow_spec_eth *>(
      flow_rule + sizeof(ibv_exp_flow_attr));
  static_assert(IBV_EXP_FLOW_SPEC_ETH == 0x20, "");
  flow_spec_eth->type = IBV_EXP_FLOW_SPEC_ETH;
  static_assert(sizeof(struct ibv_exp_flow_spec_eth) == 0x28, "");
  flow_spec_eth->size = 0x28;
  memcpy(flow_spec_eth->val.dst_mac, kDstMAC, 6);
  memset(flow_spec_eth->mask.dst_mac, 0xff, 6);

  auto *flow = ibv_exp_create_flow(cb->qp, flow_attr);
  assert(flow != nullptr);

  printf("Listening\n");
  while (true) {
    struct ibv_wc wc;
    int msgs_completed = ibv_poll_cq(cb->recv_cq, 1, &wc);
    if (msgs_completed > 0) {
      printf("message %ld received size %d\n", wc.wr_id, wc.byte_len);
      sge.addr = reinterpret_cast<uint64_t>(buf) + (wc.wr_id * kRecvBufSize);

      for (size_t i = 0; i < 60; i++) {
        printf("%02x ", reinterpret_cast<uint8_t *>(sge.addr)[i]);
      }
      printf("\n");

      wr.wr_id = wc.wr_id;
      int ret = ibv_post_recv(cb->qp, &wr, &bad_wr);
      assert(ret == 0);
    } else if (msgs_completed < 0) {
      printf("Polling error\n");
      exit(1);
    }
  }

  printf("We are done\n");
  return 0;
}
