#ifndef ICMP_UTILS_H
#define ICMP_UTILS_H

#include <stddef.h>
#include <stdint.h>

#define PACKETSIZE 576
#define IP_HDR_LEN 20
#define ICMP_HDR_LEN 8
#define ICMP_PAYLOAD_MAX (PACKETSIZE - IP_HDR_LEN - ICMP_HDR_LEN)
#define ICMP_HEADER_OFFSET 20
#define ICMP_PAYLOAD_OFFSET (ICMP_HEADER_OFFSET + ICMP_HDR_LEN)

#define ICMP_ECHO      8
#define ICMP_ECHOREPLY 0

/* Wire-format IP header (20 bytes), layout matches Linux struct iphdr */
struct ip_header {
    uint8_t  ihl_version;
    uint8_t  tos;
    uint16_t tot_len;
    uint16_t id;
    uint16_t frag_off;
    uint8_t  ttl;
    uint8_t  protocol;
    uint16_t check;
    uint32_t saddr;
    uint32_t daddr;
};

/* Wire-format ICMP header (8 bytes), layout matches Linux struct icmphdr */
struct icmp_header {
    uint8_t  type;
    uint8_t  code;
    uint16_t checksum;
    uint16_t id;
    uint16_t sequence;
};

/* Full packet for sender (IP_HDRINCL): IP header + ICMP header + payload */
struct icmp_full_packet {
    struct ip_header ip;
    struct icmp_header hdr;
    char msg[ICMP_PAYLOAD_MAX];
};

/* ICMP-only block for receiver (kernel adds IP): ICMP header + payload */
struct icmp_echo_block {
    struct icmp_header hdr;
    char payload[ICMP_PAYLOAD_MAX];
};

unsigned short checksum(void *b, int len);

int parse_icmp_payload(const char *buf, int len, char *out, size_t out_size);

int get_icmp_id_seq(const char *buf, int len, uint16_t *id, uint16_t *seq);

int build_echo_reply(const char *req_buf, int req_len, const char *payload,
                     size_t payload_len, char *out_pkt, size_t *out_len);

#endif /* ICMP_UTILS_H */
