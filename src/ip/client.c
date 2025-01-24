#include "utils.h"
#include <string.h>

struct ip *prepare_ip_header(char *datagram, const char *dest_ip,
                             const char *message) {
  struct ip *ip_header = (struct ip *)datagram;

  ip_header->ip_hl = 5;
  ip_header->ip_v = 4;
  ip_header->ip_tos = 0;
  ip_header->ip_len = htons(sizeof(struct ip) + strlen(message));
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

struct ip *prepare_ip_packet(const char *dest_ip, const char *message) {
  static char datagram[MAX_DATAGRAM_SIZE];
  explicit_bzero(datagram, MAX_DATAGRAM_SIZE);

  struct ip *ip_header = prepare_ip_header(datagram, dest_ip, message);
  size_t remaining_size = MAX_DATAGRAM_SIZE - sizeof(struct ip);
  snprintf(datagram + sizeof(struct ip), remaining_size, "%s",
           message); // Add payload
  return ip_header;
}

struct sockaddr_in prepare_dest_addr(const char *dest_ip) {
  struct sockaddr_in dest_addr;
  explicit_bzero(&dest_addr, sizeof(dest_addr));
  dest_addr.sin_family = AF_INET;

  if (inet_pton(AF_INET, dest_ip, &(dest_addr.sin_addr)) != 1) {
    perror("inet_pton failed for destination address");
    exit(EXIT_FAILURE);
  }

  return dest_addr;
}

void send_message(int sockfd, const char *dest_ip, const char *message) {
  struct ip *ip_header = prepare_ip_packet(dest_ip, message);
  char *datagram = (char *)ip_header;

  struct sockaddr_in dest_addr = prepare_dest_addr(dest_ip);

  if (sendto(sockfd, datagram, ntohs(ip_header->ip_len), 0,
             (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
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