#include "funcs.c"

int main(int argc, char **argv)
{   
    // Get host from domain
    hname = gethostbyname("127.0.0.1");
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

    char message[PACKETSIZE];
    char last_message[PACKETSIZE];

    wait_for_message(message, last_message);

    close(sockfd);
    return EXIT_SUCCESS;
}