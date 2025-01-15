#include "checksum.h"

uint16_t calculate_udp_checksum(struct ip* ip_header, struct udp_packet* udp_packet) {
    uint32_t checksum = 0;
    uint16_t* ptr;
    int length = udp_packet->header.length;
    
    // Create pseudo header
    struct {
        uint32_t src_ip;
        uint32_t dest_ip;
        uint8_t zero;
        uint8_t protocol;
        uint16_t udp_length;
    } __attribute__((packed)) pseudo_header;
    
    pseudo_header.src_ip = ip_header->ip_src.s_addr;
    pseudo_header.dest_ip = ip_header->ip_dst.s_addr;
    pseudo_header.zero = 0;
    pseudo_header.protocol = IPPROTO_UDP;
    pseudo_header.udp_length = udp_packet->header.length;
    
    // Add pseudo header to checksum
    ptr = (uint16_t*)&pseudo_header;
    for (int i = 0; i < sizeof(pseudo_header)/2; i++) {
        checksum += ntohs(ptr[i]);
    }
    
    // Add UDP header and data
    ptr = (uint16_t*)udp_packet;
    int udp_length = (length + 1) / 2; // Round up to handle odd bytes
    for (int i = 0; i < udp_length; i++) {
        checksum += ntohs(ptr[i]);
    }
    
    // Add any carry bits and get one's complement
    while (checksum >> 16) {
        checksum = (checksum & 0xFFFF) + (checksum >> 16);
    }
    
    return (checksum == 0xFFFF) ? checksum : ~checksum;
}