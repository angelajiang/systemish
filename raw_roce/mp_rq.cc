#include <assert.h>
#include <infiniband/verbs.h>
#include <infiniband/verbs_exp.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PORT_NUM 1

#define ENTRY_SIZE \
  9000  // The maximum size of each received packet - set to jumbo frame
#define RQ_NUM_DESC \
  512  // The maximum receive ring length without processing \

// The MAC we are listening to. In case your setup is via a network switch, you
// may need to change the MAC address to suit the network port MAC
#define DEST_MAC \
  { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 }

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

  struct ibv_cq *cq = ibv_create_cq(context, RQ_NUM_DESC, nullptr, nullptr, 0);
  assert(cq != nullptr);

  struct ibv_qp_init_attr qp_init_attr;
  memset(&qp_init_attr, 0, sizeof(qp_init_attr));
  qp_init_attr.qp_context = nullptr;
  qp_init_attr.send_cq = cq;
  qp_init_attr.recv_cq = cq;
  qp_init_attr.cap.max_send_wr = 0;  // No send ring
  qp_init_attr.cap.max_recv_wr = RQ_NUM_DESC;
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

  // Register RX ring memory
  size_t buf_size = ENTRY_SIZE * RQ_NUM_DESC;
  void *buf = malloc(buf_size);
  assert(buf != nullptr);

  struct ibv_mr *mr = ibv_reg_mr(pd, buf, buf_size, IBV_ACCESS_LOCAL_WRITE);
  assert(mr != nullptr);

  // Attach all buffers to the ring
  struct ibv_sge sge;
  struct ibv_recv_wr wr, *bad_wr;

  // pointer to packet buffer size and memory key of each packet buffer
  sge.length = ENTRY_SIZE;
  sge.lkey = mr->lkey;
  wr.num_sge = 1;
  wr.sg_list = &sge;
  wr.next = nullptr;

  for (size_t n = 0; n < RQ_NUM_DESC; n++) {
    sge.addr = reinterpret_cast<uint64_t>(buf) + ENTRY_SIZE * n;
    wr.wr_id = n;
    ibv_post_recv(qp, &wr, &bad_wr);
  }

  // 12. Register steering rule to intercept packet to DEST_MAC and place packet
  // in ring pointed by ->qp
  struct raw_eth_flow_attr {
    struct ibv_flow_attr attr;
    struct ibv_flow_spec_eth spec_eth;
  } __attribute__((packed)) flow_attr;

  memset(&flow_attr, 0, sizeof(flow_attr));

  flow_attr.attr.comp_mask = 0;
  flow_attr.attr.type = IBV_FLOW_ATTR_NORMAL;
  flow_attr.attr.size = sizeof(flow_attr);
  flow_attr.attr.priority = 0;
  flow_attr.attr.num_of_specs = 1;
  flow_attr.attr.port = PORT_NUM;
  flow_attr.attr.flags = 0;

  flow_attr.spec_eth.type =
      static_cast<enum ibv_flow_spec_type>(IBV_EXP_FLOW_SPEC_ETH);
  flow_attr.spec_eth.size = sizeof(struct ibv_flow_spec_eth);
  flow_attr.spec_eth.val.dst_mac = DEST_MAC;
  flow_attr.spec_eth.val.src_mac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  flow_attr.spec_eth.val.ether_type = 0;
  flow_attr.spec_eth.val.vlan_tag = 0;

  flow_attr.spec_eth.mask.src_mac = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  flow_attr.spec_eth.mask.dst_mac = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  flow_attr.spec_eth.mask.ether_type = 0;
  flow_attr.spec_eth.mask.vlan_tag = 0;

  // Create steering rule
  struct ibv_flow *eth_flow;
  eth_flow = ibv_create_flow(qp, &flow_attr.attr);
  assert(eth_flow != nullptr);

  while (true) {
    struct ibv_wc wc;
    int msgs_completed = ibv_poll_cq(cq, 1, &wc);
    if (msgs_completed > 0) {
      printf("message %ld received size %d\n", wc.wr_id, wc.byte_len);
      sge.addr = reinterpret_cast<uint64_t>(buf) + wc.wr_id * ENTRY_SIZE;
      wr.wr_id = wc.wr_id;
      ibv_post_recv(qp, &wr, &bad_wr);
    } else if (msgs_completed < 0) {
      printf("Polling error\n");
      exit(1);
    }
  }

  printf("We are done\n");
  return 0;
}
