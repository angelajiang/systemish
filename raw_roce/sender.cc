#include "common.h"

static constexpr size_t kSQDepth = 512;

int main() {
  int ret;
  struct ibv_device **dev_list = ibv_get_device_list(nullptr);
  assert(dev_list != nullptr);

  struct ibv_device *ib_dev = dev_list[0];
  assert(ib_dev != nullptr);

  struct ibv_context *context = ibv_open_device(ib_dev);
  assert(context != nullptr);

  struct ibv_pd *pd = ibv_alloc_pd(context);
  assert(pd != nullptr);

  struct ibv_cq *cq = ibv_create_cq(context, kSQDepth, nullptr, nullptr, 0);
  assert(cq != nullptr);

  struct ibv_qp_init_attr qp_init_attr;
  memset(&qp_init_attr, 0, sizeof(qp_init_attr));
  qp_init_attr.qp_context = nullptr;
  qp_init_attr.send_cq = cq;
  qp_init_attr.recv_cq = cq;
  qp_init_attr.cap.max_send_wr = kSQDepth;
  qp_init_attr.cap.max_inline_data = 512;
  qp_init_attr.cap.max_recv_wr = 0;  // No RECV ring
  qp_init_attr.cap.max_recv_sge = 1;
  qp_init_attr.qp_type = IBV_QPT_RAW_PACKET;

  struct ibv_qp *qp = ibv_create_qp(pd, &qp_init_attr);
  assert(qp != nullptr);

  // Initialize the QP and assign a port
  struct ibv_qp_attr qp_attr;
  int qp_flags;
  memset(&qp_attr, 0, sizeof(qp_attr));
  qp_flags = IBV_QP_STATE | IBV_QP_PORT;
  qp_attr.qp_state = IBV_QPS_INIT;
  qp_attr.port_num = 1;
  ret = ibv_modify_qp(qp, &qp_attr, qp_flags);
  assert(ret >= 0);

  // Move to RTR
  memset(&qp_attr, 0, sizeof(qp_attr));
  qp_flags = IBV_QP_STATE;
  qp_attr.qp_state = IBV_QPS_RTR;
  ret = ibv_modify_qp(qp, &qp_attr, qp_flags);
  assert(ret >= 0);

  // Move to RTS
  memset(&qp_attr, 0, sizeof(qp_attr));
  qp_flags = IBV_QP_STATE;
  qp_attr.qp_state = IBV_QPS_RTS;
  ret = ibv_modify_qp(qp, &qp_attr, qp_flags);
  assert(ret >= 0);

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
    ret = ibv_post_send(qp, &wr, &bad_wr);
    usleep(20000);
    if (ret < 0) {
      fprintf(stderr, "Failed in post send\n");
      exit(1);
    }

    if (nb_tx++ % 1000000 == 0) printf("Sent %zu packets\n", nb_tx);

    struct ibv_wc wc;
    int ret = ibv_poll_cq(cq, 1, &wc);
    assert(ret >= 0);
  }
  return 0;
}
