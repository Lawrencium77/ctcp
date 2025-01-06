#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg){
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, portno;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr; 
    int n;

    // Parse arguments
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    portno = atoi(argv[1]);

    // Create a UDP socket (instead of TCP)
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)  {
        error("ERROR opening socket");
    }
    
    // Setup serv_addr
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    // Bind socket to server address
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
    {
        error("ERROR on binding");
    }
    
    // Receive data (UDP is connectionless, so no listen/accept)
    clilen = sizeof(cli_addr);
    memset(buffer, 0, 256);
    n = recvfrom(sockfd, buffer, 255, 0, (struct sockaddr *) &cli_addr, &clilen);
    if (n < 0) {
        error("ERROR reading from socket");
    }
    printf("Here is the message: %s\n", buffer);

    // Send response back to client
    n = sendto(sockfd, "I got your message", 18, 0, (struct sockaddr *) &cli_addr, clilen);
    if (n < 0) {
        error("ERROR writing to socket");
    }

    close(sockfd);
    return 0; 
}
