#include "types.h"
#include "utils.h"

void print_payload(char *buffer, sockaddr_in src_addr) {
  ip *ip_header = (ip *)buffer;
  int header_len = ip_header->ip_hl * 4;

  printf("Received from %s: %s\n", inet_ntoa(src_addr.sin_addr),
         buffer + header_len);
}

void read_loop(int sockfd, sockaddr_in src_addr, socklen_t addr_len) {
  printf("Server listening...\n");

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
  }
}

int main() {
  int sockfd = create_ip_socket();
  sockaddr_in src_addr;
  socklen_t addr_len = sizeof(src_addr);

  read_loop(sockfd, src_addr, addr_len);

  close(sockfd);
  return 0;
}