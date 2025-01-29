#include "checksum.h"
#include <inttypes.h>
#include <string.h>

// See https://en.wikipedia.org/wiki/User_Datagram_Protocol#Checksum_computation
uint16_t calculate_udp_checksum(uint32_t dest_ip, udp_datagram *udp_packet) {
  uint32_t checksum = 0;
  uint16_t *ptr;
  int udp_length = udp_packet->header.length;

  udp_packet->header.checksum = 0;

  udp_pseudo_header pseudo_header;
  pseudo_header.src_ip = 0;
  pseudo_header.dest_ip = dest_ip;
  pseudo_header.zero = 0;
  pseudo_header.protocol = IPPROTO_RAW;
  pseudo_header.udp_length = udp_length;

  ptr = (uint16_t *)&pseudo_header;
  for (int i = 0; i < sizeof(pseudo_header) / 2; i++) {
    checksum += ntohs(ptr[i]);
  }

  // Account for final byte if UDP packet has odd size
  // by padding payload with one byte of zeros
  if (udp_length % 2 == 1) {
    explicit_bzero((uint8_t *)udp_packet + udp_length, 1);
  }

  ptr = (uint16_t *)udp_packet;
  int half_udp_length = (udp_length + 1) / 2; // Round up to handle odd bytes
  for (int i = 0; i < half_udp_length; i++) {
    checksum += ntohs(ptr[i]);
  }

  while (checksum >> 16) {
    checksum = (checksum & 0xFFFF) + (checksum >> 16);
  }

  return (checksum == 0xFFFF) ? checksum : ~checksum;
}