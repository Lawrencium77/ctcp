#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <destination_ip> <message>\n", argv[0]);
        exit(1);
    }

    // Create raw IP socket
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(1);
    }

    // Tell kernel we'll include our own IP header
    int one = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt failed");
        exit(1);
    }

    // Prepare IP header
    char datagram[4096];
    struct ip *ip_header = (struct ip *)datagram;
    
    // Zero out the packet buffer
    memset(datagram, 0, 4096);

    // Fill in the IP header
    ip_header->ip_hl = 5;    // 5 * 32-bit words = 20 bytes
    ip_header->ip_v = 4;     // IPv4
    ip_header->ip_tos = 0;
    ip_header->ip_len = sizeof(struct ip) + strlen(argv[2]);
    ip_header->ip_id = htons(54321);
    ip_header->ip_off = 0;
    ip_header->ip_ttl = 64;  // Time-To-Live
    ip_header->ip_p = IPPROTO_RAW;
    ip_header->ip_sum = 0;   // Kernel will fill in if left at 0
    ip_header->ip_src.s_addr = inet_addr("192.168.1.2");  // Source IP (change as needed)
    ip_header->ip_dst.s_addr = inet_addr(argv[1]);        // Destination IP

    // Add the payload (message)
    strcpy(datagram + sizeof(struct ip), argv[2]);

    // Prepare destination address
    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr(argv[1]);

    // Send the packet
    if (sendto(sockfd, datagram, ip_header->ip_len, 0,
               (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("sendto failed");
        exit(1);
    }

    printf("Sent message to %s\n", argv[1]);

    close(sockfd);
    return 0;
}