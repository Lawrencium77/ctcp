#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <unistd.h>

#include "client_ports.h"

#define PORT_MIN 49152
#define PORT_MAX 65535

// PORT_STATE_FILE is a bitmap to record which ports are in use
#define PORT_STATE_FILE "/tmp/udp_ports.state"

int get_ephemeral_port() {
  int fd = open(PORT_STATE_FILE, O_RDWR | O_CREAT, 0644);
  if (fd < 0) {
    perror("Failed to open port state file");
    exit(EXIT_FAILURE);
  }

  if (flock(fd, LOCK_EX) < 0) { // Blocking
    perror("Failed to flock port state file");
    exit(EXIT_FAILURE);
  }

  // This is conceptually a bitmap but 7 in every 8 bits are wasted.
  // TODO: Optimise this by using bitwise ops
  uint8_t ports[PORT_MAX - PORT_MIN + 1] = {0};
  ssize_t read_size = read(fd, ports, sizeof(ports));

  if (read_size == 0) {
    lseek(fd, 0, SEEK_SET);
    explicit_bzero(ports, sizeof(ports));
  }

  int port = -1;
  for (int i = 0; i < PORT_MAX - PORT_MIN + 1; i++) {
    if (!ports[i]) {
      port = PORT_MIN + i;
      ports[i] = 1;
      break;
    }
  }

  if (port == -1) {
    fprintf(stderr, "No ports available\n");
    exit(EXIT_FAILURE);
  }

  lseek(fd, 0, SEEK_SET);
  write(fd, ports, sizeof(ports));

  flock(fd, LOCK_UN);
  close(fd);

  return port;
}

void release_ephemeral_port(int port) {
  int fd = open(PORT_STATE_FILE, O_RDWR);
  if (fd < 0)
    return;

  flock(fd, LOCK_EX);

  uint16_t ports[PORT_MAX - PORT_MIN + 1];
  read(fd, ports, sizeof(ports));

  ports[port - PORT_MIN] = 0;

  lseek(fd, 0, SEEK_SET);
  write(fd, ports, sizeof(ports));

  flock(fd, LOCK_UN);
  close(fd);
}