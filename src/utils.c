#include "utils.h"

int create_ip_socket() {
  int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
  if (sockfd < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  int one = 1;
  if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
    perror("setsockopt failed");
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  return sockfd;
}