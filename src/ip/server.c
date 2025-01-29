#include "ip_socket.h"
#include "types.h"
#include <stdio.h>
#include <string.h>

void print_payload(char *buffer, sockaddr_in src_addr) {
  ip *ip_header = (ip *)buffer;
  int header_len = ip_header->ip_hl * 4;

  printf("Received from %s: %s\n", inet_ntoa(src_addr.sin_addr),
         buffer + header_len);
}

void read_loop(int sockfd) {
  printf("Server listening...\n");

  sockaddr_in src_addr;
  socklen_t addr_len = sizeof(src_addr);

  // Slight overestimate as buffer only needs to be as large as the largest
  // possible *payload*
  char buffer[MAX_DATAGRAM_SIZE];

  while (1) {
    ssize_t recv_len = recvfrom(sockfd, buffer, MAX_DATAGRAM_SIZE, 0,
                                (sockaddr *)&src_addr, &addr_len);

    if (recv_len < 0) {
      perror("recvfrom failed");
      continue;
    }

    print_payload(buffer, src_addr);
    explicit_bzero(buffer, sizeof(buffer));
  }
}

int main() {
  int sockfd = create_ip_socket();
  read_loop(sockfd);
  close(sockfd);
  return 0;
}