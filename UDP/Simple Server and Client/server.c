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

typedef struct ListNode {
    struct ListNode *next;
    struct ListNode *prev;

    struct sockaddr_in addr;
} ListNode;

typedef struct List {
    u_int32_t size;
    struct ListNode *head;
    struct ListNode *tail;
} List;

List *list_init() {
    ListNode *head = calloc(1, sizeof(ListNode));
    ListNode *tail = calloc(1, sizeof(ListNode));

    head->next = tail;
    tail ->prev = tail;

    List *list = calloc(1, sizeof(List));
    list->size = 0;
    list->head = head;
    list->tail = tail;

    return list;
}

void list_free(List *list) {
    ListNode *curr = list->head;
    while(curr != NULL) {
        ListNode *next = curr->next;

        free(curr);
        curr = next;
    }

    free(list);
}

void list_add(List *list, struct sockaddr_in addr) {
    ListNode *prev = list->tail->prev;
    ListNode *next = list->tail;

    ListNode *newNode = calloc(1, sizeof(ListNode));
    newNode->addr = addr;

    prev->next = newNode;
    newNode->prev = prev;
    newNode->next = next;
    next->prev = newNode;
    list->size++;
}

void list_remove(List *list, struct sockaddr_in addr) {
    for(ListNode *curr = list->head->next; curr != list->tail; curr = curr->next) {
        struct sockaddr_in nodeAddr = curr->addr;
        if(nodeAddr.sin_addr.s_addr == addr.sin_addr.s_addr && nodeAddr.sin_port == addr.sin_port) {
            ListNode *prev = curr->prev;
            ListNode *next = curr->next;

            prev->next = next;
            next->prev = prev;

            free(curr);
            list->size--;
            return;
        }
    }
}

ListNode *list_get(List *list, struct sockaddr_in addr) {
    for(ListNode *curr = list->head->next; curr != list->tail; curr = curr->next) {
        struct sockaddr_in nodeAddr = curr->addr;
        if(nodeAddr.sin_addr.s_addr == addr.sin_addr.s_addr && nodeAddr.sin_port == addr.sin_port) {
            return curr;
        }
    }

    return NULL;
}

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


    struct sockaddr_in myAddr;
    myAddr.sin_family = AF_INET;
    myAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, "192.168.50.20", &(myAddr.sin_addr));



    // Bind the socket's address
    int bindStatus = bind(socketFD, (struct sockaddr *) &myAddr, sizeof(myAddr));
    if(bindStatus == -1) {
        perror("Error binding socket's address");
        return -1;
    }


    char buffer[MAX_SIZE];
    char clientIP[INET_ADDRSTRLEN];
    struct sockaddr_in clientAddr;
    socklen_t clientSize = sizeof(clientAddr);
    puts("Waiting for connection");
    
    ssize_t bytes = 0;
    printf("Bytes received: %ld\n", bytes = recvfrom(socketFD, buffer, MAX_SIZE - 1,
                                0, (struct sockaddr *) &clientAddr, &clientSize));
    buffer[bytes] = '\0';
    printf("Received: %s --- From %s:%d\n", buffer, inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN), ntohs(clientAddr.sin_port));

    char *serverMessage = "Hello from Server";
    sendto(socketFD, serverMessage, strlen(serverMessage), 0, (struct sockaddr *) &clientAddr, clientSize);



    // Close the socket
    if(close(socketFD) == -1) {
        perror("Error closing socket");
            return -1;
    }

    return 0;
}
