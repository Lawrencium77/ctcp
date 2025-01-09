#include "utils.h"

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
            MAX_DATAGRAM_SIZE, 
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
    int sockfd = create_ip_socket();
    char buffer[MAX_DATAGRAM_SIZE]; // Slight overestimate as buffer only needs to be as large as the largest possible *payload*
    struct sockaddr_in src_addr;
    socklen_t addr_len = sizeof(src_addr);

    read_loop(sockfd, buffer, src_addr, addr_len);

    close(sockfd);
    return 0;
}