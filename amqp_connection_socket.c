#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

char amqpProtocolHeader[] = "AMQP\x00\x00\x09\x01";

int main () {
    int sockfd;
    struct sockaddr_in serverAddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8000);  // Replace with your AMQP server's port
    inet_pton(AF_INET, "amqp-server-ip", &(serverAddr.sin_addr));  // Replace with your AMQP server's IP address

    if (connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Connection to the server failed");
        exit(EXIT_FAILURE);
    }
}