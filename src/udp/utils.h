#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>

#define MAX_UDP_DATAGRAM_SIZE 2048
#define MAX_IP_DATAGRAM_SIZE 4096
#define PORT_NUMBER 4486 // Placeholder. TODO: Don't hard-code port number

int create_udp_socket() {
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(1);
    }

    int one = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt failed");
        exit(1);
    }

    return sockfd;
}

u_short get_checksum(const char* message) {
    unsigned long sum = 0;
    
    while (*message != '\0') {
        sum += (unsigned char)*message;
        message++;
    }
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    return (u_short)~sum;
}


