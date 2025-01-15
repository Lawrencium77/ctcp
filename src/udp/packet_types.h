#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

struct udp_header {
    uint16_t src_port;
    uint16_t dest_port;
    uint16_t length;
    uint16_t checksum;
};

struct udp_packet {
    struct udp_header header;
    char payload[];
};

#endif
