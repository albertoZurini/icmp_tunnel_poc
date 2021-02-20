#include "socket_operations.h"
#include <stdlib.h>
#include <stdio.h>

char *SOURCE_ADDRESS = "255.255.255.255";
char *DESTINATION_ADDRESS = "255.255.255.255";

int main(int argc, char** argv){

  if(argc < 2){
    printf("Usage: %s [-s|-c] [interface] [destination_addess]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int server = 0;
  if(strncmp(argv[1], "-s", strlen(argv[1])) == 0){
    server = 1;
  }
  else {
    //strcpy(DESTINATION_ADDRESS, argv[3]);
    DESTINATION_ADDRESS = argv[3];
  }

  //strcpy(SOURCE_ADDRESS, argv[2]);
  SOURCE_ADDRESS = argv[2];

  printf("local ip: %s\n", SOURCE_ADDRESS);

  // retreive local IP address
/*
  int fd;
  struct ifreq ifr;
  fd = socket(AF_INET, SOCK_DGRAM, 0);
  ifr.ifr_addr.sa_family = AF_INET; // GET IPv4
  strncpy(ifr.ifr_name, argv[2], IFNAMSIZ-1); // get IP of argv[2] interface
  ioctl(fd, SIOCGIFADDR, &ifr);
  close(fd);
  printf("Local IP: %s\n", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
  // TODO: put IP into variable
  if(strcmp(SOURCE_ADDRESS, "255.255.255.255"){
    printf("ERROR WHILE GETTING IP ADDRESS");
    exit(EXIT_FAILURE);
  }*/

  struct icmp_packet packet;
  int sock_fd;

  // opening socket
  sock_fd = open_icmp_socket();

  if(server) bind_icmp_socket(sock_fd);
  
  init_packet(sock_fd, &packet, SOURCE_ADDRESS, DESTINATION_ADDRESS);

  if(server){
    printf("Listening...\n");
    while(1){
      memset(&packet, 0, sizeof(struct icmp_packet));
      receive_icmp_packet(sock_fd, &packet);

      print_packet(&packet);
      free(packet.payload);

      flip_addresses(&packet); // for response

      set_payload(&packet, "Response from server");
      set_reply_type(&packet); // set as reply type

      print_packet(&packet);
      send_icmp_packet(sock_fd, &packet);
    }
  } else {
    printf("Sending ICMP packet");

    set_payload(&packet, "Request to server");
    set_echo_type(&packet);

    print_packet(&packet);
    send_icmp_packet(sock_fd, &packet);
    printf(" [Ok]\n");
    free(packet.payload);

    printf("Waiting for packet...\n");
    receive_icmp_packet(sock_fd, &packet);
    printf("%s\n", packet.payload);
    print_packet(&packet);
  }

  printf("\n Closing socket\n");
  close_icmp_socket(sock_fd);

  exit(EXIT_SUCCESS);
}
