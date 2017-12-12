#include <arpa/inet.h>
#include <assert.h>
#include <infiniband/verbs.h>
#include <infiniband/verbs_exp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static constexpr size_t kPortIndex = 2;       // mlx5_0
static constexpr size_t kPktSize = 60;        // Packet size, including headers
static constexpr size_t kRecvBufSize = 1500;  // RECV buffer size

static constexpr size_t kSQDepth = 512;
static constexpr size_t kRQDepth = 512;

static constexpr size_t kDeviceIndex = 2;

uint8_t kDstMAC[6] = {0xec, 0x0d, 0x9a, 0x7b, 0xd7, 0xd6};
uint8_t kDstIP[4] = {192, 168, 1, 250};

uint8_t kSrcMAC[6] = {0xec, 0x0d, 0x9a, 0x7b, 0xd7, 0xe6};
uint8_t kSrcIP[4] = {192, 168, 1, 251};

struct ctrl_blk_t {
  struct ibv_device *ib_dev;
  struct ibv_context *context;
  struct ibv_pd *pd;
  struct ibv_cq *send_cq;
  struct ibv_cq *recv_cq;
  struct ibv_qp *qp;
};

ctrl_blk_t *init_ctx(size_t device_index) {
  ctrl_blk_t *cb = new ctrl_blk_t();
  memset(cb, 0, sizeof(ctrl_blk_t));

  struct ibv_device **dev_list = ibv_get_device_list(nullptr);
  assert(dev_list != nullptr);

  cb->ib_dev = dev_list[device_index];
  assert(cb->ib_dev != nullptr);
  printf("Using device %s\n", cb->ib_dev->name);

  cb->context = ibv_open_device(cb->ib_dev);
  assert(cb->context != nullptr);

  cb->pd = ibv_alloc_pd(cb->context);
  assert(cb->pd != nullptr);

  struct ibv_exp_cq_init_attr cq_init_attr;
  memset(&cq_init_attr, 0, sizeof(cq_init_attr));

  cb->send_cq = ibv_exp_create_cq(cb->context, kSQDepth, nullptr, nullptr, 0,
                                  &cq_init_attr);
  assert(cb->send_cq != nullptr);

  cb->recv_cq = ibv_exp_create_cq(cb->context, kRQDepth, nullptr, nullptr, 0,
                                  &cq_init_attr);
  assert(cb->send_cq != nullptr);

  struct ibv_exp_qp_init_attr qp_init_attr;
  memset(&qp_init_attr, 0, sizeof(qp_init_attr));

  qp_init_attr.comp_mask =
      IBV_EXP_QP_INIT_ATTR_PD | IBV_EXP_QP_INIT_ATTR_CREATE_FLAGS;

  qp_init_attr.pd = cb->pd;
  qp_init_attr.send_cq = cb->send_cq;
  qp_init_attr.recv_cq = cb->recv_cq;
  qp_init_attr.cap.max_send_wr = kSQDepth;
  qp_init_attr.cap.max_inline_data = 512;
  qp_init_attr.cap.max_recv_wr = kRQDepth;
  qp_init_attr.cap.max_recv_sge = 1;
  qp_init_attr.qp_type = IBV_QPT_RAW_PACKET;
  qp_init_attr.exp_create_flags |= IBV_EXP_QP_CREATE_SCATTER_FCS;

  cb->qp = ibv_exp_create_qp(cb->context, &qp_init_attr);
  assert(cb->qp != nullptr);

  // Initialize the QP and assign a port
  struct ibv_exp_qp_attr qp_attr;
  memset(&qp_attr, 0, sizeof(qp_attr));
  qp_attr.qp_state = IBV_QPS_INIT;
  qp_attr.port_num = 1;
  int ret = ibv_exp_modify_qp(cb->qp, &qp_attr, IBV_QP_STATE | IBV_QP_PORT);
  assert(ret >= 0);

  // Move to RTR
  memset(&qp_attr, 0, sizeof(qp_attr));
  qp_attr.qp_state = IBV_QPS_RTR;
  ret = ibv_exp_modify_qp(cb->qp, &qp_attr, IBV_QP_STATE);
  assert(ret >= 0);

  // Move to RTS
  memset(&qp_attr, 0, sizeof(qp_attr));
  qp_attr.qp_state = IBV_QPS_RTS;
  ret = ibv_exp_modify_qp(cb->qp, &qp_attr, IBV_QP_STATE);
  assert(ret >= 0);

  return cb;
}

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

int ip_from_str(char *ip, uint32_t *addr) {
  return inet_pton(AF_INET, ip, addr);
}

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
