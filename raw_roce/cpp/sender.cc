#include <arpa/inet.h>
#include <assert.h>
#include <infiniband/verbs.h>
#include <infiniband/verbs_exp.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PORT_NUM 1
#define ENTRY_SIZE 9000  // maximum size of each send buffer
#define SQ_NUM_DESC 512  // maximum number of sends waiting for completion

struct eth_hdr_t {
  uint8_t dst_mac[6];
  uint8_t src_mac[6];
  uint16_t eth_type;
} __attribute__((packed));

struct ipv4_hdr_t {
  uint8_t ihl : 4;
  uint8_t version : 4;
  uint8_t tos;
  uint16_t tot_len;
  uint16_t id;
  uint16_t frag_off;
  uint8_t ttl;
  uint8_t protocol;
  uint16_t check;
  uint32_t saddr;
  uint32_t daddr;
} __attribute__((packed));

static uint16_t ip_checksum(void *buf, size_t hdr_len) {
  unsigned long sum = 0;
  const uint16_t *ip1 = reinterpret_cast<uint16_t *>(buf);
  while (hdr_len > 1) {
    sum += *ip1++;
    if (sum & 0x80000000) sum = (sum & 0xFFFF) + (sum >> 16);
    hdr_len -= 2;
  }
  while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);

  return (~sum);
}

void gen_ipv4_header(void *ip_header_buffer, uint32_t *saddr, uint32_t *daddr,
                     uint8_t protocol, int pkt_size) {
  ipv4_hdr_t ip_header;
  memset(&ip_header, 0, sizeof(ip_header));

  ip_header.version = 4;
  ip_header.ihl = 5;
  ip_header.tos = 0;
  ip_header.tot_len = htons(pkt_size);
  ip_header.id = htons(0);
  ip_header.frag_off = htons(0);
  ip_header.ttl = 128;
  ip_header.protocol = protocol;
  ip_header.saddr = *saddr;
  ip_header.daddr = *daddr;
  ip_header.check = ip_checksum(&ip_header, sizeof(ip_header));
  memcpy(ip_header_buffer, &ip_header, sizeof(ip_header));
}

void gen_eth_header(eth_hdr_t *eth_header, uint8_t *src_mac, uint8_t *dst_mac,
                    uint16_t eth_type) {
  memcpy(eth_header->src_mac, src_mac, 6);
  memcpy(eth_header->dst_mac, dst_mac, 6);
  eth_header->eth_type = htons(eth_type);
}

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
  // uint8_t DST_MAC[6] = {0xd6, 0xd7, 0x7b, 0x9a, 0x0d, 0xec};
  uint8_t DST_MAC[6] = {0xec, 0x0d, 0x9a, 0x7b, 0xd7, 0xd6};
  uint8_t SRC_MAC[6] = {0xe4, 0x1d, 0x2d, 0xf3, 0xdd, 0xcc};
  uint8_t ETH_TYPE[2] = {0x08, 0x00};
  uint8_t IP_HDRS[12] = {0x45, 0x00, 0x00, 0x54, 0x00, 0x00,
                         0x40, 0x00, 0x40, 0x01, 0xaf, 0xb6};
  uint8_t SRC_IP[4] = {10, 1, 1, 1};
  uint8_t DST_IP[4] = {10, 1, 1, 1};
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
