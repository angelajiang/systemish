#include <assert.h>
#include <infiniband/verbs_exp.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

static constexpr size_t kRQDepth = 512;

void copy_mac(uint8_t dst_mac[6], const uint8_t src_mac[6]) {
  for (int i = 0; i < 6; i++) dst_mac[i] = src_mac[i];
}

int main() {
  int ret;
  struct ibv_device **dev_list = ibv_get_device_list(nullptr);
  assert(dev_list != nullptr);

  struct ibv_device *ib_dev = dev_list[kPortIndex];
  assert(ib_dev != nullptr);
  printf("Using device %s\n", ib_dev->name);

  struct ibv_context *context = ibv_open_device(ib_dev);
  assert(context != nullptr);

  struct ibv_pd *pd = ibv_alloc_pd(context);
  assert(pd != nullptr);

  struct ibv_exp_cq_init_attr cq_init_attr;
  memset(&cq_init_attr, 0, sizeof(cq_init_attr));
  struct ibv_cq *cq =
      ibv_exp_create_cq(context, kRQDepth, nullptr, nullptr, 0, &cq_init_attr);
  assert(cq != nullptr);

  struct ibv_exp_qp_init_attr qp_init_attr;
  memset(&qp_init_attr, 0, sizeof(qp_init_attr));

  qp_init_attr.comp_mask =
      IBV_EXP_QP_INIT_ATTR_PD | IBV_EXP_QP_INIT_ATTR_CREATE_FLAGS;

  qp_init_attr.pd = pd;
  qp_init_attr.send_cq = cq;
  qp_init_attr.recv_cq = cq;
  qp_init_attr.cap.max_recv_wr = kRQDepth;
  qp_init_attr.cap.max_recv_sge = 1;
  qp_init_attr.qp_type = IBV_QPT_RAW_PACKET;
  qp_init_attr.cap.max_inline_data = 60;

  struct ibv_qp *qp = ibv_exp_create_qp(context, &qp_init_attr);
  assert(qp != nullptr);

  // Initialize the QP and assign a port
  struct ibv_exp_qp_attr init_attr;
  memset(&init_attr, 0, sizeof(init_attr));
  init_attr.qp_state = IBV_QPS_INIT;
  init_attr.pkey_index = 0;
  init_attr.port_num = 1;
  uint64_t init_comp_mask =
      IBV_QP_STATE | IBV_QP_PKEY_INDEX | IBV_QP_PORT | IBV_QP_QKEY;

  ret = ibv_exp_modify_qp(qp, &init_attr, init_comp_mask);
  assert(ret >= 0);

  // Move to RTR
  struct ibv_exp_qp_attr rtr_attr;
  memset(&rtr_attr, 0, sizeof(rtr_attr));
  rtr_attr.qp_state = IBV_QPS_RTR;
  ret = ibv_exp_modify_qp(qp, &rtr_attr, IBV_QP_STATE);
  assert(ret >= 0);

  // Register RX ring memory
  size_t ring_size = kRecvBufSize * kRQDepth;
  void *buf = malloc(ring_size);
  assert(buf != nullptr);

  struct ibv_mr *mr = ibv_reg_mr(pd, buf, ring_size, IBV_ACCESS_LOCAL_WRITE);
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
    ibv_post_recv(qp, &wr, &bad_wr);
  }

  // Promiscuous listening
  struct ibv_flow_attr attr;
  memset(&attr, 0, sizeof(attr));
  attr.type = IBV_FLOW_ATTR_ALL_DEFAULT;
  attr.num_of_specs = 0;
  attr.port = 1;
  attr.flags = 0;
  auto *flow = ibv_create_flow(qp, &attr);
  assert(flow != nullptr);

  printf("Listening\n");
  while (true) {
    struct ibv_wc wc;
    int msgs_completed = ibv_poll_cq(cq, 1, &wc);
    if (msgs_completed > 0) {
      printf("message %ld received size %d\n", wc.wr_id, wc.byte_len);
      sge.addr = reinterpret_cast<uint64_t>(buf) + (wc.wr_id * kRecvBufSize);

      for (size_t i = 0; i < 60; i++) {
        printf("%02x ", reinterpret_cast<uint8_t *>(sge.addr)[i]);
      }
      printf("\n");

      wr.wr_id = wc.wr_id;
      int ret = ibv_post_recv(qp, &wr, &bad_wr);
      assert(ret == 0);
    } else if (msgs_completed < 0) {
      printf("Polling error\n");
      exit(1);
    }
  }

  printf("We are done\n");
  return 0;
}

/*
// Register steering rule to intercept packet to DST_MAC and place packet
// in ring pointed by ->qp
//uint8_t DST_MAC[6] = {0xd6, 0xd7, 0x7b, 0x9a, 0x0d, 0xec};
uint8_t DST_MAC[6] = {0xec, 0x0d, 0x9a, 0x7b, 0xd7, 0xd6};
uint8_t SRC_MAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t MASK_MAC[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

constexpr size_t flow_rule_size =
    sizeof(struct ibv_flow_attr) + sizeof(ibv_flow_spec_eth);
uint8_t flow_rule[flow_rule_size];
memset(flow_rule, 0, flow_rule_size);

auto *flow_attr = reinterpret_cast<struct ibv_flow_attr *>(flow_rule);
flow_attr->comp_mask = 0;
flow_attr->type = IBV_FLOW_ATTR_NORMAL;
flow_attr->size = sizeof(flow_attr);  // XXX: Check
flow_attr->priority = 0;
flow_attr->num_of_specs = 1;
flow_attr->port = PORT_NUM;
flow_attr->flags = 0;

auto *spec_eth = reinterpret_cast<struct ibv_flow_spec_eth *>(
    flow_rule + sizeof(struct ibv_flow_attr));

spec_eth->type = static_cast<enum ibv_flow_spec_type>(IBV_EXP_FLOW_SPEC_ETH);
spec_eth->size = sizeof(struct ibv_flow_spec_eth);
copy_mac(spec_eth->val.dst_mac, DST_MAC);
copy_mac(spec_eth->val.src_mac, SRC_MAC);
spec_eth->val.ether_type = 0;
spec_eth->val.vlan_tag = 0;

copy_mac(spec_eth->mask.src_mac, MASK_MAC);
copy_mac(spec_eth->mask.dst_mac, MASK_MAC);
spec_eth->mask.ether_type = 0;
spec_eth->mask.vlan_tag = 0;

// Create steering rule
struct ibv_flow *eth_flow;
eth_flow = ibv_create_flow(qp, flow_attr);
assert(eth_flow != nullptr);
*/
