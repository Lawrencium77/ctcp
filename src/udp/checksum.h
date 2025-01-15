#include <stdint.h>
#include "packet_types.h"
#include <netinet/ip.h>

struct udp_pseudo_header {
        uint32_t src_ip;
        uint32_t dest_ip;
        uint8_t zero;
        uint8_t protocol;
        uint16_t udp_length;
    } __attribute__((packed));

uint16_t calculate_udp_checksum(struct ip* ip_header, struct udp_packet* udp_packet);