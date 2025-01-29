#include "client_utils.h"
#include "utils.h"
#include <string.h>

void send_message(int sockfd, const char *dest_ip, const char *message) {
  ip *ip_header = prepare_ip_packet(dest_ip, (void *)message, strlen(message));
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
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <destination_ip> <message>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int sockfd = create_ip_socket();
  send_message(sockfd, argv[1], argv[2]);
  close(sockfd);
  return 0;
}