#include <assert.h>
#include <infiniband/verbs.h>
#include <infiniband/verbs_exp.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PORT_NUM 1
#define ENTRY_SIZE 9000  // maximum size of each send buffer
#define SQ_NUM_DESC 512  // maximum number of sends waiting for completion

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

  struct ibv_cq *cq = ibv_create_cq(context, SQ_NUM_DESC, nullptr, nullptr, 0);
  assert(cq != nullptr);

  struct ibv_qp_init_attr qp_init_attr;
  memset(&qp_init_attr, 0, sizeof(qp_init_attr));
  qp_init_attr.qp_context = nullptr;
  qp_init_attr.send_cq = cq;
  qp_init_attr.recv_cq = cq;
  qp_init_attr.cap.max_send_wr = SQ_NUM_DESC;
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

  // Register RX ring memory
  size_t buf_size = ENTRY_SIZE * SQ_NUM_DESC;
  void *buf = malloc(buf_size);
  assert(buf != nullptr);

  struct ibv_mr *mr = ibv_reg_mr(pd, buf, buf_size, IBV_ACCESS_LOCAL_WRITE);
  assert(mr != nullptr);

  // Template of the packet to send
  uint8_t DST_MAC[6] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
  uint8_t SRC_MAC[6] = {0xe4, 0x1d, 0x2d, 0xf3, 0xdd, 0xcc};
  uint8_t ETH_TYPE[2] = {0x08, 0x00};
  uint8_t IP_HDRS[12] = {0x45, 0x00, 0x00, 0x54, 0x00, 0x00,
                         0x40, 0x00, 0x40, 0x01, 0xaf, 0xb6};
  uint8_t SRC_IP[4] = {0x0d, 0x07, 0x38, 0x66};
  uint8_t DST_IP[4] = {0x0d, 0x07, 0x38, 0x7f};
  uint8_t IP_OPT[5] = {0x08, 0x00, 0x59, 0xd0, 0x88};
  uint8_t ICMP_HDR[9] = {0x2c, 0x00, 0x09, 0x52, 0xae, 0x96, 0x57, 0x00, 0x00};

  char packet[150];
  size_t idx = 0;
  memcpy(&packet[idx], DST_MAC, 6);
  idx += 6;
  memcpy(&packet[idx], SRC_MAC, 6);
  idx += 6;
  memcpy(&packet[idx], ETH_TYPE, 2);
  idx += 2;
  memcpy(&packet[idx], IP_HDRS, 12);
  idx += 12;
  memcpy(&packet[idx], SRC_IP, 4);
  idx += 4;
  memcpy(&packet[idx], DST_IP, 4);
  idx += 4;
  memcpy(&packet[idx], IP_OPT, 5);
  idx += 5;
  memcpy(&packet[idx], ICMP_HDR, 9);
  idx += 9;

  struct ibv_sge sg_entry;
  sg_entry.addr = reinterpret_cast<uint64_t>(buf);
  sg_entry.length = sizeof(packet);
  sg_entry.lkey = mr->lkey;

  struct ibv_send_wr wr;
  memset(&wr, 0, sizeof(wr));
  wr.num_sge = 1;
  wr.sg_list = &sg_entry;
  wr.next = nullptr;
  wr.opcode = IBV_WR_SEND;

  // Do SENDS
  int n = 0;
  int msgs_completed;
  while (true) {
    wr.send_flags = IBV_SEND_INLINE;

    if ((n % (SQ_NUM_DESC / 2)) == 0) {
      wr.wr_id = static_cast<size_t>(n);
      wr.send_flags |= IBV_SEND_SIGNALED;
    }

    struct ibv_send_wr *bad_wr;
    ret = ibv_post_send(qp, &wr, &bad_wr);
    if (ret < 0) {
      fprintf(stderr, "Failed in post send\n");
      exit(1);
    }
    n++;

    // poll for completion after half ring is posted
    if ((n % (SQ_NUM_DESC / 2)) == 0 && n > 0) {
      struct ibv_wc wc;
      msgs_completed = ibv_poll_cq(cq, 1, &wc);
      if (msgs_completed > 0) {
        printf("completed message %ld\n", wc.wr_id);
      } else if (msgs_completed < 0) {
        printf("Polling error\n");
        exit(1);
      }
    }
  }

  printf("We are done\n");
  return 0;
}
