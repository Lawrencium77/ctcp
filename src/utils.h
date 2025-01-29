#ifndef UTILS_H
#define UTILS_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_DATAGRAM_SIZE 4096
#define ETH_MTU 1500 // Ethernet MTU is 1500 bytes

int create_ip_socket();

#endif
