#include "checksum.h"
#include <stdio.h>
#include <inttypes.h>

// See https://en.wikipedia.org/wiki/User_Datagram_Protocol#Checksum_computation
uint16_t calculate_udp_checksum(
    ip* ip_header, 
    udp_datagram* udp_packet
) {
    uint32_t checksum = 0;
    uint16_t* ptr;
    int length = udp_packet->header.length;

    udp_packet->header.checksum = 0;
    
    udp_pseudo_header pseudo_header;
    pseudo_header.src_ip = 0;
    pseudo_header.dest_ip = ip_header->ip_dst.s_addr;
    pseudo_header.zero = 0;
    pseudo_header.protocol = IPPROTO_RAW;
    pseudo_header.udp_length = udp_packet->header.length;

    ptr = (uint16_t*)&pseudo_header;
    for (int i = 0; i < sizeof(pseudo_header)/2; i++) {
        checksum += ntohs(ptr[i]);
    }
    
    ptr = (uint16_t*)udp_packet;
    int udp_length = (length + 1) / 2; // Round up to handle odd bytes
    for (int i = 0; i < udp_length; i++) {
        checksum += ntohs(ptr[i]);
    }
    
    while (checksum >> 16) {
        checksum = (checksum & 0xFFFF) + (checksum >> 16);
    }
    
    return (checksum == 0xFFFF) ? checksum : ~checksum;
}