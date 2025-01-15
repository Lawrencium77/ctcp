#include <stdint.h>
#include "packet_types.h"
#include <netinet/ip.h>

uint16_t calculate_udp_checksum(struct ip* ip_header, struct udp_packet* udp_packet);