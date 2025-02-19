#include "checksum.h"
#include "client_ports.h"
#include "ip_client_utils.h"
#include "ip_socket.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

udp_datagram *prepare_udp_packet(const char *message, uint32_t dest_ip,
                                 const char *dest_port, int src_port) {
  static char packet[MAX_DATAGRAM_SIZE];
  explicit_bzero(packet, MAX_DATAGRAM_SIZE);

  udp_datagram *udp_packet = (udp_datagram *)packet;
  udp_packet->header.src_port = src_port;
  udp_packet->header.dest_port = atoi(dest_port);
  udp_packet->header.length = sizeof(udp_header) + strlen(message);

  size_t payload_size = MAX_DATAGRAM_SIZE - sizeof(udp_header);
  snprintf(udp_packet->payload, payload_size, "%s", message);

  udp_packet->header.checksum = calculate_udp_checksum(dest_ip, udp_packet);

  return udp_packet;
}

void send_message(int sockfd, const char *dest_ip, const char *dest_port,
                  const char *message, int src_port) {
  sockaddr_in dest_addr = prepare_dest_addr(dest_ip);
  uint32_t dest_addr_as_int = dest_addr.sin_addr.s_addr;

  udp_datagram *udp_packet =
      prepare_udp_packet(message, dest_addr_as_int, dest_port, src_port);
  ip *ip_header =
      prepare_ip_packet(dest_ip, udp_packet, udp_packet->header.length);
  char *datagram = (char *)ip_header;

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