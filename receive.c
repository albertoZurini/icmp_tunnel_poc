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
    fprintf(stderr, "[receive] Shutting down receiver (Ctrl+C)\n");
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        if (errno == EPERM) {
            fprintf(stderr, "[receive] ERROR: Raw ICMP socket requires root. Try: sudo ./receive\n");
        } else {
            fprintf(stderr, "[receive] ERROR: socket creation failed: %s\n", strerror(errno));
        }
        return EXIT_FAILURE;
    }

    /* Do not set IP_HDRINCL so kernel fills IP header when sending echo reply */
    signal(SIGINT, on_signal);

    char buf[1024];
    char payload[ICMP_PAYLOAD_MAX + 1];
    char reply_buf[ICMP_PAYLOAD_MAX + 1];
    char reply_pkt[sizeof(struct icmp_echo_block)];
    size_t reply_len;
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);

    fprintf(stderr, "[receive] Listening for ICMP echo requests. Ctrl+C to stop.\n");

    while (running) {
        memset(buf, 0, sizeof(buf));
        len = sizeof(addr);
        ssize_t retval = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&addr, &len);

        if (retval < 0) {
            if (!running) break;
            fprintf(stderr, "[receive] ERROR: recvfrom failed: %s\n", strerror(errno));
            continue;
        }

        if ((size_t)retval < ICMP_PAYLOAD_OFFSET) {
            continue;
        }

        /* Only handle Echo Request (type 8) */
        unsigned char type = (unsigned char)buf[ICMP_HEADER_OFFSET];
        if (type != ICMP_ECHO) {
            continue;
        }

        if (parse_icmp_payload(buf, (int)retval, payload, sizeof(payload)) != 0) {
            fprintf(stderr, "[receive] ERROR: failed to parse payload\n");
            continue;
        }

        char addr_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr.sin_addr, addr_str, sizeof(addr_str));
        fprintf(stderr, "[receive] Received echo request from %s\n", addr_str);
        printf("[receive] Payload: %s\n", payload);

        printf("Reply text> ");
        fflush(stdout);
        if (fgets(reply_buf, (int)sizeof(reply_buf), stdin) == NULL) {
            if (!running) break;
            continue;
        }
        /* Trim newline */
        size_t rlen = strlen(reply_buf);
        if (rlen > 0 && reply_buf[rlen - 1] == '\n') {
            reply_buf[rlen - 1] = '\0';
            rlen--;
        }

        if (build_echo_reply(buf, (int)retval, reply_buf, rlen, reply_pkt, &reply_len) != 0) {
            fprintf(stderr, "[receive] ERROR: failed to build echo reply\n");
            continue;
        }

        if (sendto(sockfd, reply_pkt, reply_len, 0, (struct sockaddr *)&addr, len) < 0) {
            fprintf(stderr, "[receive] ERROR: sendto failed: %s\n", strerror(errno));
            continue;
        }

        fprintf(stderr, "[receive] Sent echo reply to %s\n", addr_str);
    }

    close(sockfd);
    fprintf(stderr, "[receive] Exit.\n");
    return EXIT_SUCCESS;
}
