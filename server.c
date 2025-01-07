#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

int main() {
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

    char buffer[1024];
    struct sockaddr_in src_addr;
    socklen_t addr_len = sizeof(src_addr);

    printf("Server listening...\n");

    while (1) {
        // Receive datagram
        ssize_t recv_len = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                                  (struct sockaddr*)&src_addr, &addr_len);
        
        if (recv_len < 0) {
            perror("recvfrom failed");
            continue;
        }

        // Skip IP header
        struct ip* ip_header = (struct ip*)buffer;
        int header_len = ip_header->ip_hl * 4;
        
        // Print the payload
        printf("Received from %s: %s\n", 
               inet_ntoa(src_addr.sin_addr),
               buffer + header_len);
    }

    close(sockfd);
    return 0;
}