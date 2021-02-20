#include "funcs.c"

int main(int argc, char **argv)
{   
    // Enough arguments
    if (argc < 3) {
        printf("usage: ./tracert <server> <message>\n");
        return EXIT_FAILURE;
    }
    
    char message[192]; // message to send
    int count=0;
    for(int i=2; i<argc; ++i){
      for(int j=0; j<strlen(argv[i]); ++j){
        message[count] = argv[i][j];
        ++count;
      };
      message[count] = ' ';
      ++count;
    }
    message[count-1] = '\0';
    //printf("%s\n", message);

    // Get host from domain
    hname = gethostbyname(argv[1]);
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = hname->h_addrtype;
    addr.sin_port = htons(6969);
    addr.sin_addr.s_addr = *(long *)hname->h_addr_list[0]; 

    // Create ICMP RAW socket
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        printf("error on socket creation\n");
        return EXIT_FAILURE;;
    }

    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &seq, sizeof(seq)) < 0) {
        printf("error on default ip header settings\n");
        return EXIT_FAILURE;
    }

    // Loop and receive/send packets
    while (1) {
    // Make packet
        memset(&pckt, 0, sizeof(pckt));

        // IP Header
        pckt.ip.version = 4;
        pckt.ip.ihl = 5;
        pckt.ip.tot_len = htons(sizeof(pckt));
        pckt.ip.ttl = 255;
        pckt.ip.protocol = IPPROTO_ICMP;
        pckt.ip.saddr = inet_addr("127.0.0.1");//inet_addr("192.168.1.129");
        pckt.ip.daddr = addr.sin_addr.s_addr;
        pckt.ip.check = checksum(&pckt, sizeof(struct iphdr));

        // ICMP Header
        pckt.hdr.type = ICMP_ECHO;
        pckt.hdr.un.echo.id = 0;

        // Put my message into the packet
        for(int i=0; i<strlen(message); ++i){
          pckt.msg[i] = message[i];
        }

        pckt.hdr.un.echo.sequence = seq;
        pckt.hdr.checksum = checksum(&pckt.hdr, sizeof(struct icmphdr) + sizeof(pckt.msg));

        // Send packet
        if (sendto(sockfd, &pckt, sizeof(pckt), 0, (struct sockaddr*)&addr, sizeof(addr)) <= 0) {
            printf("error on sending packet\n");
            return EXIT_FAILURE;
        }

        char message[PACKETSIZE];
        char last_message[PACKETSIZE];

        wait_for_message(message, last_message);

        // The while is there for later, for now I just want to send one packet
        return EXIT_SUCCESS;
    }

    close(sockfd);
    return EXIT_SUCCESS;
}