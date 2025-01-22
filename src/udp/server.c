#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "server_common.h"

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);

    int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket(AF_UNIX)");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un daemon_addr;
    memset(&daemon_addr, 0, sizeof(daemon_addr));
    daemon_addr.sun_family = AF_UNIX;
    strncpy(daemon_addr.sun_path, DAEMON_SOCK_PATH, sizeof(daemon_addr.sun_path) - 1);

    if (connect(sock_fd, (struct sockaddr *)&daemon_addr, sizeof(daemon_addr)) < 0) {
        perror("connect to daemon");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    char buf[MAX_UDP_PORT_LENGTH];
    snprintf(buf, sizeof(buf), "%d", port);
    ssize_t n = write(sock_fd, buf, strlen(buf));
    if (n < 0) {
        perror("write(port) to daemon");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    printf("server: Bound to port=%d (user-level). Waiting for data...\n", port);

    while (1) {
        char recv_buf[1024];
        memset(recv_buf, 0, sizeof(recv_buf));
        ssize_t r = read(sock_fd, recv_buf, sizeof(recv_buf)-1);
        if (r < 0) {
            perror("read() from daemon");
            break;
        } else if (r == 0) {
            printf("server: Daemon closed connection.\n");
            break;
        }
        printf("server(port=%d): Received: %.*s\n", port, (int)r, recv_buf);
    }

    close(sock_fd);
    return 0;
}