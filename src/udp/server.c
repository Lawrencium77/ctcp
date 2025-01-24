#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "server_common.h"
#include "utils.h"

int create_unix_domain_socket() {
  int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sock_fd < 0) {
    perror("Creation of Unix domain socket failed");
    exit(EXIT_FAILURE);
  }
  return sock_fd;
}

void connect_to_daemon(int sock_fd) {
  struct sockaddr_un daemon_addr;
  memset(&daemon_addr, 0, sizeof(daemon_addr));
  daemon_addr.sun_family = AF_UNIX;
  strncpy(daemon_addr.sun_path, DAEMON_SOCK_PATH,
          sizeof(daemon_addr.sun_path) - 1);

  if (connect(sock_fd, (struct sockaddr *)&daemon_addr, sizeof(daemon_addr)) <
      0) {
    perror("Failed to connect to daemon");
    close(sock_fd);
    exit(EXIT_FAILURE);
  }
}

void send_port_to_daemon(int sock_fd, int port) {
  char buf[MAX_UDP_PORT_LENGTH];
  snprintf(buf, sizeof(buf), "%d", port);
  ssize_t n = write(sock_fd, buf, strlen(buf));
  if (n < 0) {
    perror("Failed to send port number to daemon");
    close(sock_fd);
    exit(EXIT_FAILURE);
  }
}

void receive_ack_from_daemon(int sock_fd, int port) {
  int reply;
  ssize_t daemon_response = read(sock_fd, &reply, sizeof(reply)); // Blocking
  if (daemon_response <= 0) {
    fprintf(stderr, "Daemon closed connection or read error.\n");
    close(sock_fd);
    exit(EXIT_FAILURE);
  }

  if (reply != 0) {
    fprintf(stderr, "Port %d is already in use\n", port);
    close(sock_fd);
    exit(EXIT_FAILURE);
  }

  printf("server: Bound to port=%d (user-level). Waiting for data...\n", port);
}

void handle_new_data(int port, char *recv_buf, ssize_t r) {
  if (r < 0) {
    perror("read() from daemon");
    return;
  } else if (r == 0) {
    printf("server: Daemon closed connection.\n");
    return;
  }
  printf("server(port=%d): Received: %.*s\n", port, (int)r, recv_buf);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int port = atoi(argv[1]);
  int sock_fd = create_unix_domain_socket();

  // The server and daemon processes interact with a double-handshake during
  // startup
  connect_to_daemon(sock_fd);
  send_port_to_daemon(sock_fd, port);
  receive_ack_from_daemon(sock_fd, port);

  while (1) {
    char recv_buf[MAX_DATAGRAM_SIZE];
    memset(recv_buf, 0, sizeof(recv_buf));
    ssize_t r = read(sock_fd, recv_buf, sizeof(recv_buf) - 1); // Blocking
    handle_new_data(port, recv_buf, r);
    if (r <= 0) {
      break;
    }
  }

  close(sock_fd);
  return 0;
}