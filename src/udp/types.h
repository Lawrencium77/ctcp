#ifndef TYPES_H
#define TYPES_H

#include <netinet/ip.h>
#include <stdint.h>

typedef struct ip ip;
typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;

typedef struct {
  uint16_t src_port;
  uint16_t dest_port;
  uint16_t length;
  uint16_t checksum;
} udp_header;

typedef struct {
  udp_header header;
  char payload[];
} udp_datagram;

typedef struct {
  int port;
  int fd;
} port_map_entry;

// The daemon handles most of the packet handling complexity and passes
// the source port and received message to the server. In the future it
// might make more sense to pass the entire UDP packet to the server.
typedef struct {
  int port;
  char payload[];
} daemon_to_server_packet;

#endif
