#include "utils.h"

void print_payload(
    char* buffer,
    struct sockaddr_in src_addr
){
    // Skip IP header
    struct ip* ip_header = (struct ip*)buffer;
    int header_len = ip_header->ip_hl * 4;

    // Skip UDP header
    struct udphdr* udp_header = (struct udphdr*)(buffer + header_len); 
    header_len += sizeof(struct udphdr);

    // Checksum verification
    size_t message_length = udp_header->uh_ulen - sizeof(struct udphdr);
    u_short checksum_host_order = get_checksum(buffer + header_len);
    u_short checksum_network_order = htons(checksum_host_order);
    if (checksum_network_order != udp_header->uh_sum) {
        printf("Checksum verification failed\n");
    }
    else {
        printf("Received from %s: %s\n", 
            inet_ntoa(src_addr.sin_addr),
            buffer + header_len);
    }
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
            MAX_IP_DATAGRAM_SIZE, 
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
    int sockfd = create_udp_socket();
    char buffer[MAX_IP_DATAGRAM_SIZE];
    struct sockaddr_in src_addr;
    socklen_t addr_len = sizeof(src_addr);

    read_loop(sockfd, buffer, src_addr, addr_len);

    close(sockfd);
    return 0;
}