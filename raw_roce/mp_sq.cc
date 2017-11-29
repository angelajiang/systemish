#include <assert.h>
#include <infiniband/verbs.h>
#include <infiniband/verbs_exp.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PORT_NUM 1
#define ENTRY_SIZE 9000  // maximum size of each send buffer
#define SQ_NUM_DESC 512  // maximum number of sends waiting for completion

// template of packet to send - in this case icmp
#define DST_MAC 0x00, 0x01, 0x02, 0x03, 0x04, 0x05
#define SRC_MAC 0xe4, 0x1d, 0x2d, 0xf3, 0xdd, 0xcc
#define ETH_TYPE 0x08, 0x00
#define IP_HDRS \
  0x45, 0x00, 0x00, 0x54, 0x00, 0x00, 0x40, 0x00, 0x40, 0x01, 0xaf, 0xb6
#define SRC_IP 0x0d, 0x07, 0x38, 0x66
#define DST_IP 0x0d, 0x07, 0x38, 0x7f
#define IP_OPT 0x08, 0x00, 0x59, 0xd0, 0x88
#define ICMP_HDR 0x2c, 0x00, 0x09, 0x52, 0xae, 0x96, 0x57, 0x00, 0x00

char packet[] = {
    DST_MAC, SRC_MAC, ETH_TYPE, IP_HDRS, SRC_IP, DST_IP, IP_OPT, ICMP_HDR, 0x00,
    0x00,    0x62,    0x21,     0x0c,    0x00,   0x00,   0x00,   0x00,     0x00,
    0x10,    0x11,    0x12,     0x13,    0x14,   0x15,   0x16,   0x17,     0x18,
    0x19,    0x1a,    0x1b,     0x1c,    0x1d,   0x1e,   0x1f,   0x20,     0x21,
    0x22,    0x23,    0x24,     0x25,    0x26,   0x27,   0x28,   0x29,     0x2a,
    0x2b,    0x2c,    0x2d,     0x2e,    0x2f,   0x30,   0x31,   0x32,     0x33,
    0x34,    0x35,    0x36,     0x37};

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

  memcpy(buf, packet, sizeof(packet));

  struct ibv_sge sg_entry;
  struct ibv_send_wr wr, *bad_wr;
  int msgs_completed;
  struct ibv_wc wc;

  sg_entry.addr = reinterpret_cast<uint64_t>(buf);
  sg_entry.length = sizeof(packet);
  sg_entry.lkey = mr->lkey;

  memset(&wr, 0, sizeof(wr));

  wr.num_sge = 1;
  wr.sg_list = &sg_entry;
  wr.next = nullptr;
  wr.opcode = IBV_WR_SEND;

  // Do SENDS
  int n = 0;
  while (true) {
    wr.send_flags = IBV_SEND_INLINE;

    if ((n % (SQ_NUM_DESC / 2)) == 0) {
      wr.wr_id = static_cast<size_t>(n);
      wr.send_flags |= IBV_SEND_SIGNALED;
    }

    ret = ibv_post_send(qp, &wr, &bad_wr);
    if (ret < 0) {
      fprintf(stderr, "Failed in post send\n");
      exit(1);
    }
    n++;

    // poll for completion after half ring is posted
    if ((n % (SQ_NUM_DESC / 2)) == 0 && n > 0) {
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
