#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <errno.h>
#include <fcntl.h>

#include "checksum.h"
#include "types.h"
#include "server_common.h"
#include "utils.h"

#define MAX_SERVERS 10

static port_map_entry port_map[MAX_SERVERS];
static int server_count = 0;

int create_unix_listen_socket(const char *path) {
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("Creation of Unix domain socket failed");
        exit(EXIT_FAILURE);
    }
    unlink(path);

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
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
    return -1;
}

void handle_new_server(int listen_fd) {
    struct sockaddr_un addr;
    socklen_t addr_len = sizeof(addr);
    int server_fd = accept(listen_fd, (struct sockaddr*)&addr, &addr_len);
    if (server_fd < 0) {
        perror("accept() failed");
        return;
    }

    char buf[MAX_UDP_PORT_LENGTH];
    memset(buf, 0, sizeof(buf));
    ssize_t n = read(server_fd, buf, sizeof(buf)-1);
    if (n <= 0) {
        perror("read() from server failed");
        close(server_fd);
        return;
    }

    int port = atoi(buf);
    printf("Received new server at port %d\n", port);
    add_port_mapping(port, server_fd);
}

int validate_udp_checksum(ip* ip_header, udp_datagram* udp_packet) {
    uint16_t client_checksum = udp_packet->header.checksum;
    uint16_t server_checksum = calculate_udp_checksum(ip_header, udp_packet);

    if (client_checksum == server_checksum) {
        return 0;
    } else {
        return 1;
    }
}

void handle_new_data(int raw_fd) {
    char buffer[MAX_DATAGRAM_SIZE];
    memset(buffer, 0, sizeof(buffer));
    struct sockaddr_in src_addr;
    socklen_t src_len = sizeof(src_addr);
    ssize_t recv_len = recvfrom(raw_fd, buffer, sizeof(buffer), 0,
                                (struct sockaddr*)&src_addr, &src_len);
    if (recv_len <= 0) {
        if (recv_len < 0) {
            perror("Failed raw_fd recvfrom");
        }
        return;
    }

    ip* ip_header = (ip*)buffer;
    int ip_header_len = ip_header->ip_hl * 4;
    if (recv_len < ip_header_len + (ssize_t)sizeof(udp_header)) {
        fprintf(stderr, "Recevied IP Datagram too small\n");
        return;
    }

    udp_datagram* udp_packet = (udp_datagram*)(buffer + ip_header_len);
    if (validate_udp_checksum(ip_header, udp_packet) != 0) {
        printf("Invalid UDP checksum, dropping packet\n");
        return;
    }

    int dest_port = udp_packet->header.dest_port;
    int server_fd = find_server_fd_for_port(dest_port);
    if (server_fd < 0) {
        printf("No server found for port %d, dropping packet\n", dest_port);
        return;
    }

    size_t udp_header_len = sizeof(udp_header);
    size_t payload_len = udp_packet->header.length - udp_header_len;
    if (payload_len > 0) {
        printf("Writing to server\n");
        ssize_t sent = write(server_fd, udp_packet->payload, payload_len);
        if (sent < 0) {
            perror("write() to server failed");
        }
    }
}

void cleanup(int raw_fd, int listen_fd) {
    close(raw_fd);
    close(listen_fd);
    for (int i = 0; i < server_count; i++) {
        close(port_map[i].fd);
    }
    unlink(DAEMON_SOCK_PATH);
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
            } 
            else if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("Read error on server socket");
                printf("Removing server on port %d due to error.\n", port_map[i].port);
                remove_port_mapping(i);
                continue;
            }
        }
        i++;
    }
}

int main(void) {
    int raw_fd = create_ip_socket();
    int listen_fd = create_unix_listen_socket(DAEMON_SOCK_PATH);
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
            handle_new_server(listen_fd);
        }

        if (FD_ISSET(raw_fd, &readfds)) {
            handle_new_data(raw_fd);
        }

        check_for_closed_connections(readfds);
    }

    cleanup(raw_fd, listen_fd);

    return 0;
}