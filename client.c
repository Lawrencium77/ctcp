#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#define MAX_DATAGRAM_SIZE 4096

struct ip* prepare_ip_header(
    char* datagram,
    const char* dest_ip,
    const char* message
) {
    struct ip *ip_header = (struct ip *)datagram; // ip_header points to same addr as datagram

    ip_header->ip_hl = 5;
    ip_header->ip_v = 4;
    ip_header->ip_tos = 0;
    ip_header->ip_len = sizeof(struct ip) + strlen(message);
    ip_header->ip_id = htons(54321);
    ip_header->ip_off = 0;
    ip_header->ip_ttl = 64;
    ip_header->ip_p = IPPROTO_RAW;
    ip_header->ip_sum = 0;
    ip_header->ip_src.s_addr = inet_addr("localhost");
    ip_header->ip_dst.s_addr = inet_addr(dest_ip);

    return ip_header;
}

struct ip* prepare_ip_packet(
    const char* dest_ip,
    const char* message
){
    static char datagram[MAX_DATAGRAM_SIZE];
    memset(datagram, 0, MAX_DATAGRAM_SIZE);
    
    struct ip* ip_header = prepare_ip_header(datagram, dest_ip, message);
    strcpy(datagram + sizeof(struct ip), message);  // Add datagram payload
    return ip_header;
}

struct sockaddr_in prepare_dest_addr(
    const char* dest_ip
) {
    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr(dest_ip);
    return dest_addr;
}

void send_message(
    int sockfd,
    const char* dest_ip, 
    const char* message
) {
    struct ip* ip_header = prepare_ip_packet(dest_ip, message);
    char* datagram = (char*)ip_header;

    struct sockaddr_in dest_addr = prepare_dest_addr(dest_ip);
    
    if (sendto(
            sockfd,  
            datagram, 
            ip_header->ip_len, 
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

    send_message(sockfd, argv[1], argv[2]);
    close(sockfd);
    return 0;
}