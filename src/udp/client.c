#include "checksum.h"
#include "packet_types.h"
#include "utils.h"
#include <string.h>

struct udp_packet* prepare_udp_packet(
    const char* message,
    const char* dest_port
) {
    static char packet[MAX_DATAGRAM_SIZE];
    memset(packet, 0, MAX_DATAGRAM_SIZE);

    struct udp_packet* udp_packet = (struct udp_packet*)packet;
    udp_packet->header.src_port = 0;
    udp_packet->header.dest_port = atoi(dest_port);
    udp_packet->header.length = sizeof(struct udp_header) + strlen(message);
    udp_packet->header.checksum = 0;

    strcpy(udp_packet->payload, message);
    return udp_packet;
}

struct ip* prepare_ip_header(
    char* datagram,
    const char* dest_ip,
    struct udp_packet* udp_packet
) {
    struct ip *ip_header = (struct ip *)datagram;
    uint16_t udp_packet_length = udp_packet->header.length;

    ip_header->ip_hl = 5;
    ip_header->ip_v = 4;
    ip_header->ip_tos = 0;
    ip_header->ip_len = htons(sizeof(struct ip) + udp_packet_length);
    ip_header->ip_id = htons(54321);
    ip_header->ip_off = htons(0);
    ip_header->ip_ttl = 64;
    ip_header->ip_p = IPPROTO_RAW;
    ip_header->ip_sum = 0;

    if (inet_pton(AF_INET, "0.0.0.0", &(ip_header->ip_src)) != 1) {
        perror("inet_pton failed for 0.0.0.0");
        exit(1);
    }
    
    if (inet_pton(AF_INET, dest_ip, &(ip_header->ip_dst)) != 1) {
        perror("inet_pton failed for destination address");
        exit(1);
    }
    
    return ip_header;
}

void set_udp_checksum(
    struct ip* ip_header,
    struct udp_packet* udp_packet
) {
    udp_packet->header.checksum = calculate_udp_checksum(ip_header, udp_packet);
}

struct ip* prepare_ip_packet(
    const char* dest_ip,
    struct udp_packet* udp_packet
){
    static char datagram[MAX_DATAGRAM_SIZE];
    memset(datagram, 0, MAX_DATAGRAM_SIZE);
    
    struct ip* ip_header = prepare_ip_header(datagram, dest_ip, udp_packet);
    set_udp_checksum(ip_header, udp_packet);
    memcpy(datagram + sizeof(struct ip), udp_packet, udp_packet->header.length);   // Add payload

    return ip_header;
}

struct sockaddr_in prepare_dest_addr(const char* dest_ip) {
    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    
    if (inet_pton(AF_INET, dest_ip, &(dest_addr.sin_addr)) != 1) {
        perror("inet_pton failed for destination address");
        exit(1);
    }
    
    return dest_addr;
}

void send_message(
    int sockfd,
    const char* dest_ip, 
    const char* dest_port,
    const char* message
) {
    struct udp_packet* udp_packet = prepare_udp_packet(message, dest_port);
    struct ip* ip_header = prepare_ip_packet(dest_ip, udp_packet);
    char* datagram = (char*)ip_header;

    struct sockaddr_in dest_addr = prepare_dest_addr(dest_ip);
    
    if (sendto(
            sockfd,  
            datagram, 
            ntohs(ip_header->ip_len),
            0,
            (struct sockaddr *)&dest_addr, 
            sizeof(dest_addr)
        ) < 0
    ) {
        perror("sendto failed");
        exit(1);
    }

    printf("Sent message to %s\n", dest_ip);

}


int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <destination_ip> <dest_port> <message>\n", argv[0]);
        exit(1);
    }

    int sockfd = create_ip_socket();
    send_message(sockfd, argv[1], argv[2], argv[3]);
    close(sockfd);
    return 0;
}