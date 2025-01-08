#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int create_socket() {
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
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

void print_payload(
    char* buffer,
    struct sockaddr_in src_addr
){
    // Skip header
    struct ip* ip_header = (struct ip*)buffer;
    int header_len = ip_header->ip_hl * 4;
    
    printf("Received from %s: %s\n", 
            inet_ntoa(src_addr.sin_addr),
            buffer + header_len);
}

void read_loop(
    int sockfd,
    char* buffer,
    struct sockaddr_in src_addr,
    socklen_t addr_len
) {
    printf("Server listening...\n");
    
    while (1) {
        ssize_t recv_len = recvfrom(
            sockfd, 
            buffer, 
            BUFFER_SIZE, 
            0,
            (struct sockaddr*)&src_addr, 
            &addr_len
        );
        
        if (recv_len < 0) {
            perror("recvfrom failed");
            continue;
        }

        print_payload(buffer, src_addr);
    }
}

int main() {
    int sockfd = create_socket();
    char buffer[BUFFER_SIZE];
    struct sockaddr_in src_addr;
    socklen_t addr_len = sizeof(src_addr);

    read_loop(sockfd, buffer, src_addr, addr_len);

    close(sockfd);
    return 0;
}