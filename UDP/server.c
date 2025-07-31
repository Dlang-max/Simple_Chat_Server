#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define CLIENT_LIST_SIZE 5
#define PORT 6969
#define MAX_MESSAGE_LENGTH 256
#define SERVER_ADDR "192.168.50.20"

typedef struct ClientListNode {
    struct sockaddr_in clientAddr;
} ClientListNode;


typedef struct ClientList {
    int size;
    ClientListNode *clients;
} ClientList;

ClientList *client_list_init() {
    ClientListNode *clients = calloc(CLIENT_LIST_SIZE, sizeof(ClientListNode));

    ClientList *clientList = calloc(1, sizeof(ClientList));
    clientList->size = 0;
    clientList->clients = clients;

    return clientList;
}

void client_list_free(ClientList *clientList) {
    free(clientList->clients);
    free(clientList);
}

bool client_list_contains(ClientList *clientList, struct sockaddr_in clientAddr) {
    for(int i = 0; i < clientList->size; i++) {
        long newClientAddr = clientAddr.sin_addr.s_addr;
        short newClientPort = clientAddr.sin_port;

        long storedAddr = clientList->clients[i].clientAddr.sin_addr.s_addr;
        short storedPort = clientList->clients[i].clientAddr.sin_port;

        if(newClientAddr == storedAddr && newClientPort == storedPort) {
            return true;
        }
    }

    return false;
}

void client_list_insert(ClientList *clientList, struct sockaddr_in clientAddr) {
    if(clientList->size == CLIENT_LIST_SIZE) {
        return;
    }

    clientList->clients[clientList->size++].clientAddr = clientAddr;
}


void update_clients(ClientList *clientList, struct sockaddr_in clientAddr) {
    if(client_list_contains(clientList, clientAddr)) {
        return;
    }

    client_list_insert(clientList, clientAddr);
}

void send_message_to_clients(int socketFD, ClientList *clientList, char *buffer) {
    for(int i = 0; i < clientList->size; i++) {
        struct sockaddr_in clientAddr = clientList->clients[i].clientAddr;
        sendto(socketFD, buffer, MAX_MESSAGE_LENGTH, 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
    }
}


int main(void) {
    int socketFD = socket(AF_INET, SOCK_DGRAM, 0);
    if(socketFD == -1) {
        perror("Error getting socket file descriptor");
        return 1;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_ADDR, &(serverAddr.sin_addr));

    int bindingStatus = bind(socketFD, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if(bindingStatus == -1) {
        perror("Error binding socket to address and port");
        return 1;
    }

    struct sockaddr_in clientAddr;
    socklen_t clientAddrSize;

    ClientList *clientList = client_list_init();
    printf("Running Server...\n");
    while(true) {
        char buffer[MAX_MESSAGE_LENGTH + 1];
        memset(buffer, '\0', MAX_MESSAGE_LENGTH + 1);

        // Receive data from clients
        recvfrom(socketFD, buffer, MAX_MESSAGE_LENGTH, 0, (struct sockaddr *)&clientAddr, &clientAddrSize);

        // Print message information to terminal
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
        printf("%s:%d > %s\n", clientIP, ntohs(clientAddr.sin_port), buffer);
        update_clients(clientList, clientAddr);


        // Send data to clients
        send_message_to_clients(socketFD, clientList, buffer);
    }

    free(clientList);
    if(close(socketFD) == -1) {
        perror("Error closing socket");
        return 1;
    }

    return 0;
}
