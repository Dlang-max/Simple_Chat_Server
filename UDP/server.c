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
    struct sockaddr addr;
} ListNode;

typedef struct List {
    u_int32_t size;
    struct ListNode *head;
    struct ListNode *tail;
} List;


List *list_init() {
    ListNode *head = (ListNode *)calloc(1, sizeof(ListNode));
    ListNode *tail = (ListNode *)calloc(1, sizeof(ListNode));

    head->next = tail;
    head->prev = NULL;

    tail->next = NULL;
    tail-> prev = head;

    List *list = (List *)calloc(1, sizeof(List));
    list->size = 0;
    list->head = head;
    list->tail = tail;

    return list;
}

void list_insert(List *list, struct sockaddr addr) {
    ListNode *newNode = (ListNode *)calloc(1, sizeof(ListNode));
    newNode->addr= addr;

    ListNode *head = list->head;
    ListNode *tail = list->tail;

    ListNode *temp = tail->prev;
    temp->next = newNode;
    newNode->prev = temp;
    newNode->next = tail;
    tail->prev = newNode;
}

void list_free(List *list) {
    puts("Freeing List of IP addresses");
    ListNode *curr = list->head;
    while(curr != NULL) {
        ListNode *next = curr->next;
        free(curr);
        curr = next;
    }

    free(list);
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
    while (true) {
        recvfrom(socketFD, buffer, MAX_SIZE, int flags, (struct sockaddr *) &myAddr, sizeof(myAddr));
    }








    // Close the socket
    if(close(socketFD) == -1) {
        perror("Error closing socket");
            return -1;
    }

    return 0;
}
