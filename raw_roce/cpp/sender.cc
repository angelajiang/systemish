#include <arpa/inet.h>
#include <assert.h>
#include <infiniband/verbs.h>
#include <infiniband/verbs_exp.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

static constexpr size_t kSQDepth = 512;

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
    if (ret < 0) {
      fprintf(stderr, "Failed in post send\n");
      exit(1);
    }

    if (nb_tx++ % 1000000 == 0) printf("Sent %zu packets\n", nb_tx);

    struct ibv_wc wc;
    int ret = ibv_poll_cq(cq, 1, &wc);
    assert(ret == 0);
  }
  return 0;
}
