#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];

    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    
    portno = atoi(argv[2]);
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    // Create a UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    
    // Setup serv_addr
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    // For UDP, we do not connect; we just use sendto/recvfrom
    printf("Please enter the message: ");
    memset(buffer, 0, 256);
    fgets(buffer, 255, stdin);

    // Send message to server
    n = sendto(sockfd, buffer, strlen(buffer), 0,
               (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (n < 0) 
        error("ERROR writing to socket");
    
    // Receive message from server
    socklen_t servlen = sizeof(serv_addr);
    memset(buffer, 0, 256);
    n = recvfrom(sockfd, buffer, 255, 0,
                 (struct sockaddr *)&serv_addr, &servlen);
    if (n < 0) 
        error("ERROR reading from socket");

    printf("%s\n", buffer);
    close(sockfd);
    return 0;
}
