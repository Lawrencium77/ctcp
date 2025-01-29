#ifndef CLIENT_H
#define CLIENT_H

#include "types.h"

ip *prepare_ip_header(char *datagram, const char *dest_ip, size_t payload_len);
ip *prepare_ip_packet(const char *dest_ip, void *payload, size_t payload_len);
sockaddr_in prepare_dest_addr(const char *dest_ip);

#endif