#include "checksum.h"
#include "types.h"
#include "utils.h"

int validate_udp_checksum(
    ip* ip_header,
    udp_datagram* udp_packet
) {
    uint16_t client_checksum = udp_packet->header.checksum;
    uint16_t server_checksum = calculate_udp_checksum(ip_header, udp_packet);

    if (client_checksum == server_checksum) {
        printf("Checksums match\n");
        return 0;
    } else {
        printf("Checksums do not match\n");
        return 1;
    }
}

static void print_payload(
    char* buffer,
    sockaddr_in src_addr
){
    ip* ip_header = (ip*)buffer;
    int ip_header_len = ip_header->ip_hl * 4;

    udp_datagram* udp_packet = (udp_datagram*)(buffer + ip_header_len);
    int valid_checkum = validate_udp_checksum(ip_header, udp_packet);

    if (valid_checkum == 0) {
        printf(
            "Received from %s: %s\n", 
            inet_ntoa(src_addr.sin_addr),
            buffer + ip_header_len + sizeof(udp_header)
        );
    };
}

static void read_loop(
    int sockfd,
    sockaddr_in src_addr,
    socklen_t addr_len
) {
    printf("Server listening...\n");

    // Slight overestimate as buffer only needs to be as large as the largest possible *payload*
    char buffer[MAX_DATAGRAM_SIZE];
    
    while (1) {
        ssize_t recv_len = recvfrom(
            sockfd, 
            buffer, 
            MAX_DATAGRAM_SIZE, 
            0,
            (sockaddr*)&src_addr, 
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
    sockaddr_in src_addr;
    socklen_t addr_len = sizeof(src_addr);

    read_loop(sockfd, src_addr, addr_len);

    close(sockfd);
    return 0;
}