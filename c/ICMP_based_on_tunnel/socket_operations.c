#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <netdb.h>
#include <pwd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "icmp.h"

void print_packet(struct icmp_packet *packet){
  printf("[ICMP PACKET]\n");
  printf("src: %s\n", packet->src_addr);
  printf("dest: %s\n", packet->dest_addr);
  printf("payload_size: %d\n", packet->payload_size);
  printf("payload: %s\n", packet->payload);
  printf("type: %d\n", packet->type);
}

void init_packet(int sock_fd, struct icmp_packet *packet, char *SOURCE_ADDRESS, char *DESTINATION_ADDRESS){
  // prepare packet
  memset(&packet, 0, sizeof(struct icmp_packet)); // init packet

  if (sizeof(SOURCE_ADDRESS) > sizeof(packet->src_addr)){
    perror("Lack of space: size of SOURCE_ADDRESS > size of src_addr\n");
    close(sock_fd);
    exit(EXIT_FAILURE);
  }
  strncpy(packet->src_addr, SOURCE_ADDRESS, strlen(SOURCE_ADDRESS) + 1); // set source address

  if ((strlen(DESTINATION_ADDRESS) + 1) > sizeof(packet->dest_addr)){
    perror("Lack of space for copy size of SOURCE_ADDRESS > size of dest_addr\n");
    close(sock_fd);
    exit(EXIT_FAILURE);
  }
  strncpy(packet->dest_addr, DESTINATION_ADDRESS, strlen(DESTINATION_ADDRESS) + 1); // set destination address

  packet->payload = calloc(MTU, sizeof(uint8_t));
  if (packet->payload == NULL){
    perror("No memory available\n");
    exit(EXIT_FAILURE);
  }

  packet->payload_size = MTU*sizeof(uint8_t);
  if(packet->payload_size  == -1) {
    perror("Error while reading from tun device\n");
    exit(EXIT_FAILURE);
  }
}

void set_payload(struct icmp_packet *packet, char *payload){
  strcpy(packet->payload, payload);
}

void flip_addresses(struct icmp_packet *packet){
  char src_tmp[100];
  strcpy(src_tmp, packet->src_addr);
  strcpy(packet->src_addr, packet->dest_addr);
  strcpy(packet->dest_addr, src_tmp);
}