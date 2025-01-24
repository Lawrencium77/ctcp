#include "types.h"
#include <netinet/ip.h>
#include <stdint.h>

typedef struct __attribute__((packed)) {
  uint32_t src_ip;
  uint32_t dest_ip;
  uint8_t zero;
  uint8_t protocol;
  uint16_t udp_length;
} udp_pseudo_header;

uint16_t calculate_udp_checksum(ip *ip_header, udp_datagram *udp_packet);