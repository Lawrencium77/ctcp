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
    // Initialise variables
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr; // Declares two objects of type struct sockaddr_in
    int n;

    
    // Parse arguments
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    else {
        portno = atoi(argv[1]);
    }

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // AF_INET for IPv4
    if (sockfd < 0)  {
        error("ERROR opening socket");
    }
    
    // Setup serv_addr object
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    // Bind socket to server address
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
    {
        error("ERROR on binding");
    }
    
    // Mark socket as passive (i.e. accepts incoming requests)
    listen(sockfd,5);
    
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, 
                (struct sockaddr *) &cli_addr, 
                &clilen);
    printf("Got here\n");
    if (newsockfd < 0) 
        error("ERROR on accept");


    // Read data from socket into buffer
    memset(&buffer, 0, 256);
    n = read(newsockfd,buffer,255);
    if (n < 0) {
        error("ERROR reading from socket");
    }
    printf("Here is the message: %s\n",buffer);
    n = write(newsockfd,"I got your message",18);
    if (n < 0) {
        error("ERROR writing to socket");
    }

    // Program cleanup
    close(newsockfd);
    close(sockfd);
    return 0; 
}