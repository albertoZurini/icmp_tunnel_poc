#ifndef socket_operations
#define socket_operations

#include "icmp.h"

void print_packet(struct icmp_packet *packet);

void init_packet(int sock_fd, struct icmp_packet *packet, char *SOURCE_ADDRESS, char *DESTINATION_ADDRESS);

void set_payload(struct icmp_packet *packet, char *payload);

void flip_addresses(struct icmp_packet *packet);

#include "socket_operations.c"
#endif
