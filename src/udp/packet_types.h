#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

typedef struct {
    uint16_t src_port;
    uint16_t dest_port;
    uint16_t length;
    uint16_t checksum;
} udp_header;

typedef struct{
    udp_header header;
    char payload[];
} udp_datagram;

#endif
