#include <assert.h>
#include <infiniband/verbs_exp.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

void install_flow_rule(struct ibv_qp *qp) {
  static constexpr size_t rule_sz =
      sizeof(ibv_exp_flow_attr) + sizeof(ibv_exp_flow_spec_eth) +
      sizeof(ibv_exp_flow_spec_ipv4_ext) + sizeof(ibv_exp_flow_spec_tcp_udp);

  uint8_t *flow_rule = new uint8_t[rule_sz];
  memset(flow_rule, 0, rule_sz);
  uint8_t *buf = flow_rule;

  auto *flow_attr = reinterpret_cast<struct ibv_exp_flow_attr *>(flow_rule);
  flow_attr->type = IBV_EXP_FLOW_ATTR_NORMAL;
  flow_attr->size = rule_sz;
  flow_attr->priority = 0;
  flow_attr->num_of_specs = 3;
  flow_attr->port = 1;
  flow_attr->flags = 0;
  flow_attr->reserved = 0;
  buf += sizeof(struct ibv_exp_flow_attr);

  // Ethernet
  auto *eth_spec = reinterpret_cast<struct ibv_exp_flow_spec_eth *>(buf);
  eth_spec->type = IBV_EXP_FLOW_SPEC_ETH;
  eth_spec->size = sizeof(struct ibv_exp_flow_spec_eth);
  memcpy(eth_spec->val.dst_mac, kDstMAC, 6);
  memset(eth_spec->mask.dst_mac, 0xff, 6);
  buf += sizeof(struct ibv_exp_flow_spec_eth);

  // IPv4
  auto *spec_ipv4 = reinterpret_cast<struct ibv_exp_flow_spec_ipv4_ext *>(buf);
  spec_ipv4->type = IBV_EXP_FLOW_SPEC_IPV4_EXT;
  spec_ipv4->size = sizeof(struct ibv_exp_flow_spec_ipv4_ext);
  spec_ipv4->val.dst_ip = ip_from_str(kDstIP);
  spec_ipv4->mask.dst_ip = 0xffffffffu;
  buf += sizeof(struct ibv_exp_flow_spec_ipv4_ext);

  // UDP
  auto *udp_spec = reinterpret_cast<struct ibv_exp_flow_spec_tcp_udp *>(buf);
  udp_spec->type = IBV_EXP_FLOW_SPEC_UDP;
  udp_spec->size = sizeof(struct ibv_exp_flow_spec_tcp_udp);
  udp_spec->val.dst_port = htons(kDstPort);
  udp_spec->mask.dst_port = 0xffffu;

  auto *flow = ibv_exp_create_flow(qp, flow_attr);
  assert(flow != nullptr);
}

int main() {
  ctrl_blk_t *cb = init_ctx(kDeviceIndex);
  install_flow_rule(cb->qp);

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

  printf("Listening\n");
  while (true) {
    struct ibv_wc wc;
    int msgs_completed = ibv_poll_cq(cb->recv_cq, 1, &wc);
    assert(msgs_completed >= 0);
    if (msgs_completed == 0) continue;

    printf("Message %ld received size %d\n", wc.wr_id, wc.byte_len);
    sge.addr = reinterpret_cast<uint64_t>(buf) + (wc.wr_id * kRecvBufSize);

    for (size_t i = 0; i < 60; i++) {
      printf("%02x ", reinterpret_cast<uint8_t *>(sge.addr)[i]);
    }
    printf("\n");

    wr.wr_id = wc.wr_id;
    int ret = ibv_post_recv(cb->qp, &wr, &bad_wr);
    assert(ret == 0);
  }

  printf("We are done\n");
  return 0;
}
