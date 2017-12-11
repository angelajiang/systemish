#include <assert.h>
#include <infiniband/verbs_exp.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

static constexpr size_t kRQDepth = 512;

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
  qp_init_attr.exp_create_flags |= IBV_EXP_QP_CREATE_SCATTER_FCS;

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

  auto *flow = ibv_exp_create_flow(qp, flow_attr);
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
