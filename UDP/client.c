#include <bits/posix2_lim.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT 6969
#define MAX_SIZE 1024

// Code for implementing a simple UDP based chat server
int main(int argc, char *argv[]) {
    // Get the file descriptor for the UDP socket
    int socketFD = socket(AF_INET, SOCK_DGRAM, 0);
    if(socketFD == -1) {
        perror("Error getting socket file descriptor");
        return -1;
    }

    // Create struct that holds server address info
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, "192.168.50.20", &(serverAddr.sin_addr));


    char buffer[MAX_SIZE];
    char *clientMessage = "Hello from Client";
    puts("Sending Message to Client");
    sendto(socketFD, clientMessage, strlen(clientMessage), 0, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

    struct sockaddr_in recvAddr;
    socklen_t recvSize = sizeof(recvAddr);
    char recvIP[INET_ADDRSTRLEN];
    
    ssize_t bytes = 0;
    printf("Bytes received: %ld\n", bytes = recvfrom(socketFD, buffer, MAX_SIZE - 1, 0, (struct sockaddr *) &recvAddr, &recvSize));
    buffer[bytes] = '\0';
    printf("Received: %s --- From %s:%d\n", buffer, inet_ntop(AF_INET, &recvAddr.sin_addr, recvIP, INET_ADDRSTRLEN), ntohs(recvAddr.sin_port));

    // Close the socket
    if(close(socketFD) == -1) {
        perror("Error closing socket");
            return -1;
    }

    return 0;
}
