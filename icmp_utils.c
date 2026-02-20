#include "icmp_utils.h"
#include <string.h>

unsigned short checksum(void *b, int len)
{
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2) {
        sum += *buf++;
    }
    if (len == 1) {
        sum += *(unsigned char *)buf;
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = (unsigned short)~sum;

    return result;
}

int parse_icmp_payload(const char *buf, int len, char *out, size_t out_size)
{
    if (buf == NULL || out == NULL || out_size == 0) {
        return -1;
    }
    if (len < (int)ICMP_PAYLOAD_OFFSET) {
        return -1;
    }

    size_t payload_len = (size_t)(len - ICMP_PAYLOAD_OFFSET);
    if (payload_len >= out_size) {
        payload_len = out_size - 1;
    }
    memcpy(out, buf + ICMP_PAYLOAD_OFFSET, payload_len);
    out[payload_len] = '\0';
    return 0;
}

int get_icmp_id_seq(const char *buf, int len, uint16_t *id, uint16_t *seq)
{
    if (buf == NULL || id == NULL || seq == NULL) {
        return -1;
    }
    int min_len = ICMP_HEADER_OFFSET + 8;
    if (len < min_len) {
        return -1;
    }

    const char *icmp = buf + ICMP_HEADER_OFFSET;
    *id = *(const uint16_t *)(icmp + 4);
    *seq = *(const uint16_t *)(icmp + 6);
    return 0;
}

int build_echo_reply(const char *req_buf, int req_len, const char *payload,
                     size_t payload_len, char *out_pkt, size_t *out_len)
{
    if (req_buf == NULL || out_pkt == NULL || out_len == NULL) {
        return -1;
    }
    if (req_len < ICMP_HEADER_OFFSET + 8) {
        return -1;
    }

    if (payload_len > ICMP_PAYLOAD_MAX) {
        payload_len = ICMP_PAYLOAD_MAX;
    }

    struct icmp_echo_block *block = (struct icmp_echo_block *)out_pkt;
    memset(block, 0, sizeof(*block));
    block->hdr.type = ICMP_ECHOREPLY;
    block->hdr.code = 0;
    /* Copy id and sequence from request (bytes 24-27) so byte order is preserved */
    memcpy(&block->hdr.id, req_buf + ICMP_HEADER_OFFSET + 4, 4);
    if (payload != NULL && payload_len > 0) {
        memcpy(block->payload, payload, payload_len);
    }
    block->hdr.checksum = 0;
    block->hdr.checksum = checksum(&block->hdr, ICMP_HDR_LEN + (int)payload_len);

    *out_len = ICMP_HDR_LEN + payload_len;
    return 0;
}
