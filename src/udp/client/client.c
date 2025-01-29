#include "checksum.h"
#include "client_ports.h"
#include "types.h"
#include "utils.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

udp_datagram *prepare_udp_packet(const char *message, const char *dest_port,
                                 int src_port) {
  static char packet[MAX_DATAGRAM_SIZE];
  explicit_bzero(packet, MAX_DATAGRAM_SIZE);

  udp_datagram *udp_packet = (udp_datagram *)packet;
  udp_packet->header.src_port = src_port;
  udp_packet->header.dest_port = atoi(dest_port);
  udp_packet->header.length = sizeof(udp_header) + strlen(message);
  udp_packet->header.checksum = 0;

  size_t payload_size = MAX_DATAGRAM_SIZE - sizeof(udp_header);
  snprintf(udp_packet->payload, payload_size, "%s", message);
  return udp_packet;
}

ip *prepare_ip_header(char *datagram, const char *dest_ip,
                      udp_datagram *udp_packet) {
  ip *ip_header = (ip *)datagram;
  uint16_t udp_packet_length = udp_packet->header.length;

  // Since we construct our own IP headers, the kernel will not do
  // fragmentation/reassembly of IP datagrams. We there must make sure we don't
  // exceed the link-layer MTU.
  unsigned short ip_len = sizeof(ip) + udp_packet_length;
  if (ip_len > ETH_MTU) {
    fprintf(stderr,
            "Attempting to send packet larger than MTU. Packet length: %d, "
            "MTU: %d\n",
            ip_len, ETH_MTU);
    exit(EXIT_FAILURE);
  }

  ip_header->ip_hl = 5;
  ip_header->ip_v = 4;
  ip_header->ip_tos = 0;
  ip_header->ip_len = htons(ip_len);
  ip_header->ip_id = htons(54321);
  ip_header->ip_off = htons(0);
  ip_header->ip_ttl = 64;
  ip_header->ip_p = IPPROTO_RAW;
  ip_header->ip_sum = 0;

  if (inet_pton(AF_INET, "0.0.0.0", &(ip_header->ip_src)) != 1) {
    perror("inet_pton failed for 0.0.0.0");
    exit(EXIT_FAILURE);
  }

  if (inet_pton(AF_INET, dest_ip, &(ip_header->ip_dst)) != 1) {
    perror("inet_pton failed for destination address");
    exit(EXIT_FAILURE);
  }

  return ip_header;
}

void set_udp_checksum(ip *ip_header, udp_datagram *udp_packet) {
  udp_packet->header.checksum = calculate_udp_checksum(ip_header, udp_packet);
}

ip *prepare_ip_packet(const char *dest_ip, udp_datagram *udp_packet) {
  static char datagram[MAX_DATAGRAM_SIZE];
  explicit_bzero(datagram, MAX_DATAGRAM_SIZE);

  ip *ip_header = prepare_ip_header(datagram, dest_ip, udp_packet);
  set_udp_checksum(ip_header, udp_packet);
  memcpy(datagram + sizeof(ip), udp_packet,
         udp_packet->header.length); // Add payload

  return ip_header;
}

sockaddr_in prepare_dest_addr(const char *dest_ip) {
  sockaddr_in dest_addr;
  explicit_bzero(&dest_addr, sizeof(dest_addr));
  dest_addr.sin_family = AF_INET;

  if (inet_pton(AF_INET, dest_ip, &(dest_addr.sin_addr)) != 1) {
    perror("inet_pton failed for destination address");
    exit(EXIT_FAILURE);
  }

  return dest_addr;
}

void send_message(int sockfd, const char *dest_ip, const char *dest_port,
                  const char *message, int src_port) {
  udp_datagram *udp_packet = prepare_udp_packet(message, dest_port, src_port);
  ip *ip_header = prepare_ip_packet(dest_ip, udp_packet);
  char *datagram = (char *)ip_header;

  sockaddr_in dest_addr = prepare_dest_addr(dest_ip);

  if (sendto(sockfd, datagram, ntohs(ip_header->ip_len), 0,
             (sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
    perror("sendto failed");
    exit(EXIT_FAILURE);
  }

  printf("Sent message to %s\n", dest_ip);
}

int main(int argc, char *argv[]) {
  if (argc != 4 && argc != 5) {
    fprintf(stderr,
            "Usage: %s <destination_ip> <dest_port> <message> [optional "
            "<src_port>] \n",
            argv[0]);
    exit(EXIT_FAILURE);
  }

  int sockfd = create_ip_socket();
  int src_port = (argc == 4) ? get_ephemeral_port() : atoi(argv[4]);
  printf("Running client with ephemeral port %d\n", src_port);
  send_message(sockfd, argv[1], argv[2], argv[3], src_port);
  release_ephemeral_port(src_port);
  close(sockfd);
  return 0;
}