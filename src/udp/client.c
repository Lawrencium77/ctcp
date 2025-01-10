#include "utils.h"

struct udphdr* prepare_udp_header(
    char* datagram,
    const char* message

) {
    struct udphdr* udp_header = (struct udphdr*)datagram;

    udp_header->uh_sport = PORT_NUMBER; 
    udp_header->uh_dport = PORT_NUMBER; 
    udp_header->uh_ulen = sizeof(struct udphdr) + strlen(message);
    udp_header->uh_sum = get_checksum(message);

    return udp_header;
}

struct ip* prepare_ip_header(
    char* datagram,
    const char* dest_ip,
    const struct udphdr* udp_datagram
) {
    struct ip *ip_header = (struct ip *)datagram;

    ip_header->ip_hl = 5;
    ip_header->ip_v = 4;
    ip_header->ip_tos = 0;
    ip_header->ip_len = sizeof(struct ip) + udp_datagram->uh_ulen;
    ip_header->ip_id = htons(54321);
    ip_header->ip_off = 0;
    ip_header->ip_ttl = 64;
    ip_header->ip_p = IPPROTO_UDP;
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

struct udphdr* prepare_udp_packet(
    const char* message
) {
    static char datagram[MAX_UDP_DATAGRAM_SIZE];
    memset(datagram, 0, MAX_UDP_DATAGRAM_SIZE);
    
    struct udphdr* udp_datagram = prepare_udp_header(datagram, message); // udp_datagram and datagram ptrs point to the same address
    strcpy(datagram + sizeof(struct udphdr), message);
    return udp_datagram;
}

struct ip* prepare_ip_packet(
    const char* dest_ip,
    const struct udphdr* udp_datagram
){
    static char datagram[MAX_IP_DATAGRAM_SIZE];
    memset(datagram, 0, MAX_IP_DATAGRAM_SIZE);
    
    struct ip* ip_datagram = prepare_ip_header(datagram, dest_ip, udp_datagram);
    memcpy(datagram + sizeof(struct ip), udp_datagram, udp_datagram->uh_ulen);
    return ip_datagram;
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
    const char* message
) {
    struct udphdr* udp_datagram = prepare_udp_packet(message);
    struct ip* ip_datagram = prepare_ip_packet(dest_ip, udp_datagram);
    char* datagram = (char*)ip_datagram;

    struct sockaddr_in dest_addr = prepare_dest_addr(dest_ip);
    
    if (sendto(
            sockfd,  
            datagram, 
            ip_datagram->ip_len, 
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
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <destination_ip> <message>\n", argv[0]);
        exit(1);
    }

    int sockfd = create_udp_socket();
    send_message(sockfd, argv[1], argv[2]);
    close(sockfd);
    return 0;
}