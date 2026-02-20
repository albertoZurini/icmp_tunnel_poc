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
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; /* no SA_RESTART: let recvfrom/fgets return EINTR on Ctrl+C */
    sigaction(SIGINT, &sa, NULL);

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
            if (errno == EINTR || !running) break;
            fprintf(stderr, "[receive] ERROR: recvfrom failed: %s\n", strerror(errno));
            continue;
        }

        if ((size_t)retval < ICMP_PAYLOAD_OFFSET) {
            fprintf(stderr, "[receive] DEBUG: ignoring short packet (%zd bytes, need >= %d)\n", (size_t)retval, ICMP_PAYLOAD_OFFSET);
            continue;
        }

        /* Only handle Echo Request (type 8) */
        unsigned char type = (unsigned char)buf[ICMP_HEADER_OFFSET];
        if (type != ICMP_ECHO) {
            fprintf(stderr, "[receive] DEBUG: ignoring non-echo packet (type=%u)\n", type);
            continue;
        }

        if (parse_icmp_payload(buf, (int)retval, payload, sizeof(payload)) != 0) {
            fprintf(stderr, "[receive] ERROR: failed to parse payload (packet len=%zd)\n", (size_t)retval);
            continue;
        }

        size_t payload_len = (size_t)(retval - ICMP_PAYLOAD_OFFSET);
        char addr_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr.sin_addr, addr_str, sizeof(addr_str));
        fprintf(stderr, "[receive] Received echo request from %s (packet=%zd bytes, payload=%zu bytes)\n", addr_str, (size_t)retval, payload_len);
        fprintf(stderr, "[receive] Message: %s\n", payload);
        printf("[receive] Payload: %s\n", payload);

        printf("Reply text> ");
        fflush(stdout);
        if (fgets(reply_buf, (int)sizeof(reply_buf), stdin) == NULL) {
            if (errno == EINTR || !running) break;
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

        fprintf(stderr, "[receive] Sending reply (%zu bytes) to %s\n", reply_len, addr_str);
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
