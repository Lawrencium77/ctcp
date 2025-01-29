#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#include "checksum.h"
#include "ip_socket.h"
#include "server_common.h"
#include "types.h"

#define MAX_SERVERS 10

static port_map_entry port_map[MAX_SERVERS];
static int server_count = 0;
static int raw_fd;
static int listen_fd;

int create_unix_listen_socket() {
  struct stat st;
  if (stat(DAEMON_SOCK_PATH, &st) == 0) {
    fprintf(stderr, "Socket file %s already exists\n", DAEMON_SOCK_PATH);
    exit(EXIT_FAILURE);
  }

  int fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (fd < 0) {
    perror("Creation of Unix domain socket failed");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_un addr;
  explicit_bzero(&addr, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, DAEMON_SOCK_PATH, sizeof(addr.sun_path) - 1);

  if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("Binding Unix domain socket failed");
    close(fd);
    exit(EXIT_FAILURE);
  }

  if (listen(fd, 5) < 0) {
    perror("Listening to Unix domain socket failed");
    close(fd);
    exit(EXIT_FAILURE);
  }
  return fd;
}

void add_port_mapping(int port, int server_fd) {
  if (server_count < MAX_SERVERS) {
    port_map[server_count].port = port;
    port_map[server_count].fd = server_fd;
    server_count++;
  } else {
    fprintf(stderr, "Max server count has been reached.\n");
  }
}

void remove_port_mapping(int index) {
  if (index < 0 || index >= server_count) {
    fprintf(stderr, "Invalid index %d for port mapping removal\n", index);
    return;
  }

  close(port_map[index].fd);
  memmove(&port_map[index], &port_map[index + 1],
          (server_count - index - 1) * sizeof(port_map_entry));
  server_count--;
}

int find_server_fd_for_port(int port) {
  for (int i = 0; i < server_count; i++) {
    if (port_map[i].port == port) {
      return port_map[i].fd;
    }
  }
  return 0;
}

int send_server_acknowledgement(int port, int server_fd) {
  int already_used_fd = find_server_fd_for_port(port);
  write(server_fd, &already_used_fd, sizeof(already_used_fd));
  if (already_used_fd > 0) {
    fprintf(stderr, "Port %d already in use\n", port);
    close(server_fd);
    return -1;
  }
  return 0;
}

void handle_new_server() {
  struct sockaddr_un addr;
  socklen_t addr_len = sizeof(addr);
  int server_fd = accept(listen_fd, (struct sockaddr *)&addr, &addr_len);
  if (server_fd < 0) {
    perror("accept() failed");
    return;
  }

  char buf[MAX_UDP_PORT_LENGTH];
  explicit_bzero(buf, sizeof(buf));
  ssize_t n = read(server_fd, buf, sizeof(buf) - 1);
  if (n <= 0) {
    perror("read() from server failed");
    close(server_fd);
    return;
  }

  int port = atoi(buf);
  printf("Received server connection request at port %d\n", port);
  if (send_server_acknowledgement(port, server_fd) == 0) {
    printf("New server connected on port %d\n", port);
    add_port_mapping(port, server_fd);
  }
}

int validate_udp_checksum(uint32_t dest_ip, udp_datagram *udp_packet) {
  uint16_t client_checksum = udp_packet->header.checksum;
  uint16_t server_checksum = calculate_udp_checksum(dest_ip, udp_packet);

  if (client_checksum == server_checksum) {
    return 0;
  } else {
    return 1;
  }
}

void handle_new_data() {
  char buffer[MAX_DATAGRAM_SIZE];
  struct sockaddr_in src_addr;
  socklen_t src_len = sizeof(src_addr);
  ssize_t recv_len = recvfrom(raw_fd, buffer, sizeof(buffer), 0,
                              (struct sockaddr *)&src_addr, &src_len);
  if (recv_len <= 0) {
    if (recv_len < 0) {
      perror("Failed raw_fd recvfrom");
    }
    return;
  }

  ip *ip_header = (ip *)buffer;
  int ip_header_len = ip_header->ip_hl * 4;
  if (recv_len < ip_header_len + (ssize_t)sizeof(udp_header)) {
    fprintf(stderr, "Recevied IP Datagram too small\n");
    return;
  }

  udp_datagram *udp_packet = (udp_datagram *)(buffer + ip_header_len);
  if (validate_udp_checksum(ip_header->ip_dst.s_addr, udp_packet) != 0) {
    printf("Invalid UDP checksum, dropping packet\n");
    return;
  }

  int dest_port = udp_packet->header.dest_port;
  int server_fd = find_server_fd_for_port(dest_port);
  if (server_fd == 0) {
    printf("No server found for port %d, dropping packet\n", dest_port);
    return;
  }

  size_t udp_header_len = sizeof(udp_header);
  size_t payload_len = udp_packet->header.length - udp_header_len;

  daemon_to_server_packet packet;
  packet.port = udp_packet->header.src_port;
  memcpy(packet.payload, udp_packet->payload, payload_len);

  if (payload_len > 0) {
    printf("Writing to server\n");
    ssize_t sent = write(server_fd, &packet, sizeof(int) + payload_len);
    if (sent < 0) {
      perror("write() to server failed");
    }
  }
}

void cleanup() {
  close(raw_fd);
  close(listen_fd);
  for (int i = 0; i < server_count; i++) {
    close(port_map[i].fd);
  }
  unlink(DAEMON_SOCK_PATH);
  exit(EXIT_SUCCESS);
}

void check_for_closed_connections(fd_set readfds) {
  for (int i = 0; i < server_count;) {
    int sfd = port_map[i].fd;
    if (FD_ISSET(sfd, &readfds)) {
      char tmpbuf[1];

      ssize_t n = recv(sfd, tmpbuf, sizeof(tmpbuf), MSG_PEEK);
      if (n == 0) {
        printf("Server on port %d has closed. Removing from table.\n",
               port_map[i].port);
        remove_port_mapping(i);
        continue;
      } else if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        perror("Read error on server socket");
        printf("Removing server on port %d due to error.\n", port_map[i].port);
        remove_port_mapping(i);
        continue;
      }
    }
    i++;
  }
}

void register_signal_handler() {
  struct sigaction sa;
  explicit_bzero(&sa, sizeof(sa));
  sa.sa_handler = cleanup;
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGABRT, &sa, NULL);
}

int main(void) {
  register_signal_handler();
  raw_fd = create_ip_socket();
  listen_fd = create_unix_listen_socket();
  printf("Listening on Unix socket path=%s\n", DAEMON_SOCK_PATH);

  int max_fd = (raw_fd > listen_fd ? raw_fd : listen_fd);

  while (1) {
    // Create set of file descriptors to monitor.
    // One for IP socket, one for listening Unix domain socket,
    // and one for each server.
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(raw_fd, &readfds);
    FD_SET(listen_fd, &readfds);
    for (int i = 0; i < server_count; i++) {
      int sfd = port_map[i].fd;
      FD_SET(sfd, &readfds);
      if (sfd > max_fd) {
        max_fd = sfd;
      }
    }

    for (int i = 0; i < server_count; i++) {
      int sfd = port_map[i].fd;
      if (sfd > max_fd) {
        max_fd = sfd;
      }
    }

    // Blocks until there's activity on one of the file descriptors.
    int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
    if (activity < 0) {
      if (errno == EINTR) {
        continue;
      }
      perror("select() failed");
      break;
    }

    if (FD_ISSET(listen_fd, &readfds)) {
      handle_new_server();
    }

    if (FD_ISSET(raw_fd, &readfds)) {
      handle_new_data();
    }

    check_for_closed_connections(readfds);
  }

  return 0;
}