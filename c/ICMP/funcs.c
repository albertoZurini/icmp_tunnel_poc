#ifndef func_c
#define func_c

#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <time.h> 

#define PACKETSIZE 576

void delay(int milli_seconds) 
{
  milli_seconds *= 1000;

  // Stroing start time 
  clock_t start_time = clock(); 

  // looping till required time is not acheived 
  while (clock() < start_time + milli_seconds) 
      ; 
}

// Variables
struct hostent *hname;
struct sockaddr_in addr;
int sockfd, seq = 1;
struct packet pckt;
socklen_t len;
char buf[1024];

// Packet struct
struct packet {
    struct iphdr ip; // IP Header
    struct icmphdr hdr; // ICMP Header
    char msg[PACKETSIZE - sizeof(struct icmphdr) - sizeof(struct iphdr)]; // Message
};

// Checksum function
unsigned short checksum(void *b, int len) { 
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2) {
        sum += *buf++;
    }
    if (len == 1) sum += *(unsigned char*)buf;

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;

    return result;
}

void print_sender_ip_address(int sockfd, struct sockaddr* addr, socklen_t* len){
  // not working
  //if(getsockname(sockfd, addr, len) != 0){
  //  printf("Can't get IP address");
  //  return;
  //}
  switch (addr->sa_family)
  {
      case AF_INET:
          // use ((struct sockaddr_in*)&from) as needed...
          printf("IPv4");
          break;
      case AF_INET6:
          // use ((struct sockaddr_in6*)&from) as needed...
          printf("IPv6");
          break;
  }
  printf("\n%ld - %s\n", sizeof(addr->sa_data), addr->sa_data);

}

int wait_for_message(char* message, char* last_message){
  // Loop and receive/send packets
  // Receive packet
  len = sizeof(addr);
  memset(buf, 0, sizeof(buf));
  while (1) {
      int retval = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&addr, &len);//recvmsg(sockfd, &received_message, 0);

      if(retval > 0){
        int count=0;
        for(int i=28; i<retval; i++){
          //printf("%c", buf[i]);
          message[count] = buf[i];
          ++count;
        }
        printf("Message received with length: %d bytes\n", retval);
        memcpy(last_message, message, sizeof(message));
        print_sender_ip_address(sockfd, (struct sockaddr*)&addr, &len);
        printf("%s\n", message);
        return 0;

      } else if(retval == -1){
        printf("Something went wrong");
        return -1;
      }

      // The while is there for later, for now I just want to send one packet
      //return EXIT_SUCCESS;
  }
}

#endif