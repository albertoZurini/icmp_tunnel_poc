#include "icmp_utils.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static volatile int running = 1;

static void on_signal(int sig)
{
    (void)sig;
    running = 0;
    fprintf(stderr, "[send] Shutting down (Ctrl+C)\n");
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s <server_ip_or_host>\n", argv[0]);
        return EXIT_FAILURE;
    }

    struct hostent *hname = gethostbyname(argv[1]);
    if (hname == NULL) {
        fprintf(stderr, "[send] ERROR: unknown host: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = hname->h_addrtype;
    addr.sin_addr.s_addr = *(in_addr_t *)hname->h_addr_list[0];

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        if (errno == EPERM) {
            fprintf(stderr, "[send] ERROR: Raw ICMP socket requires root. Try: sudo %s %s\n", argv[0], argv[1]);
        } else {
            fprintf(stderr, "[send] ERROR: socket creation failed: %s\n", strerror(errno));
        }
        return EXIT_FAILURE;
    }

    int one = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        fprintf(stderr, "[send] ERROR: setsockopt IP_HDRINCL failed: %s\n", strerror(errno));
        close(sockfd);
        return EXIT_FAILURE;
    }

    /* Timeout so we can re-check running and don't block forever */
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    signal(SIGINT, on_signal);

    char addr_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, addr_str, sizeof(addr_str));
    fprintf(stderr, "[send] Sending to %s. Ctrl+C to stop.\n", addr_str);

    uint16_t seq = 1;
    char message[ICMP_PAYLOAD_MAX + 1];
    char buf[1024];
    char payload[ICMP_PAYLOAD_MAX + 1];
    struct sockaddr_in from;
    socklen_t fromlen = sizeof(from);

    while (running) {
        printf("Message> ");
        fflush(stdout);
        if (fgets(message, (int)sizeof(message), stdin) == NULL) {
            if (!running) break;
            continue;
        }
        size_t mlen = strlen(message);
        if (mlen > 0 && message[mlen - 1] == '\n') {
            message[mlen - 1] = '\0';
            mlen--;
        }

        struct icmp_full_packet pckt;
        memset(&pckt, 0, sizeof(pckt));

        pckt.ip.ihl_version = 0x45;
        pckt.ip.tos = 0;
        pckt.ip.tot_len = htons(sizeof(pckt));
        pckt.ip.id = 0;
        pckt.ip.frag_off = 0;
        pckt.ip.ttl = 255;
        pckt.ip.protocol = 1; /* IPPROTO_ICMP */
        pckt.ip.saddr = 0;
        pckt.ip.daddr = addr.sin_addr.s_addr;
        pckt.ip.check = 0;
        pckt.ip.check = checksum(&pckt.ip, IP_HDR_LEN);

        pckt.hdr.type = ICMP_ECHO;
        pckt.hdr.code = 0;
        pckt.hdr.id = 0;
        pckt.hdr.sequence = htons(seq);
        if (mlen > 0) {
            memcpy(pckt.msg, message, mlen > ICMP_PAYLOAD_MAX ? ICMP_PAYLOAD_MAX : mlen);
        }
        pckt.hdr.checksum = 0;
        pckt.hdr.checksum = checksum(&pckt.hdr, ICMP_HDR_LEN + (int)ICMP_PAYLOAD_MAX);

        ssize_t sent = sendto(sockfd, &pckt, sizeof(pckt), 0, (struct sockaddr *)&addr, sizeof(addr));
        if (sent <= 0) {
            fprintf(stderr, "[send] ERROR: sendto failed: %s\n", strerror(errno));
            continue;
        }
        fprintf(stderr, "[send] Sent echo request to %s\n", addr_str);

        while (running) {
            memset(buf, 0, sizeof(buf));
            fromlen = sizeof(from);
            ssize_t n = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&from, &fromlen);
            if (n < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    fprintf(stderr, "[send] No reply (timeout). Try again.\n");
                } else {
                    fprintf(stderr, "[send] ERROR: recvfrom failed: %s\n", strerror(errno));
                }
                break;
            }
            if ((size_t)n < ICMP_PAYLOAD_OFFSET) continue;
            unsigned char type = (unsigned char)buf[ICMP_HEADER_OFFSET];
            if (type != ICMP_ECHOREPLY) continue;
            if (from.sin_addr.s_addr != addr.sin_addr.s_addr) continue;

            if (parse_icmp_payload(buf, (int)n, payload, sizeof(payload)) == 0) {
                fprintf(stderr, "[send] Received echo reply from %s\n", addr_str);
                printf("[send] Reply: %s\n", payload);
            }
            break;
        }

        seq++;
    }

    close(sockfd);
    fprintf(stderr, "[send] Exit.\n");
    return EXIT_SUCCESS;
}
