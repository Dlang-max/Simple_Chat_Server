#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

typedef struct HashEntry {
    struct sockaddr_in addr;
    struct HashEntry *next;
} HashEntry;

typedef struct HashTable {
    u_int32_t size;
    u_int32_t elements;

    HashEntry **entries;
} HashTable;

HashTable *ht_init() {
    HashEntry **entries = (HashEntry **)calloc(100, sizeof(HashEntry *));

    HashTable *ht = (HashTable *)calloc(1, sizeof(HashTable));
    ht->size = 100;
    ht->elements = 0;
    ht->entries = entries;

    return ht;
}

uint32_t hash(in_addr_t addr, in_port_t port) {
    uint32_t hostAddr = ntohl(addr);
    u_int16_t hostPort = ntohs(port);

    return (hostAddr ^ hostPort);
}

void ht_insert(HashTable *ht, struct sockaddr_in addr) {
    u_int32_t index = hash(addr.sin_addr.s_addr, addr.sin_port) % ht->size;

    HashEntry *newEntry = (HashEntry *)calloc(1, sizeof(HashEntry));
    newEntry->addr = addr;
    newEntry->next = ht->entries[index];

    ht->entries[index] = newEntry;
    ht->elements++;
}

HashEntry *ht_find(HashTable *ht, struct sockaddr_in addr) {
    u_int32_t index = hash(addr.sin_addr.s_addr, addr.sin_port) % ht->size;

    HashEntry *curr = ht->entries[index];
    while(curr != NULL) {
        if(curr->addr.sin_addr.s_addr == addr.sin_addr.s_addr && curr->addr.sin_port == addr.sin_port) {
            return curr;
        }
        curr = curr->next;
    }

    return NULL;
}

void ht_remove(HashTable *ht, struct sockaddr_in addr) {
    u_int32_t index = hash(addr.sin_addr.s_addr, addr.sin_port) % ht->size;

    HashEntry *prev = NULL;
    HashEntry *curr = ht->entries[index];
    while(curr != NULL) {
        if(curr->addr.sin_addr.s_addr == addr.sin_addr.s_addr && curr->addr.sin_port == addr.sin_port) {

            if(prev == NULL) {
                ht->entries[index] = curr->next;
            } else {
                prev->next = curr->next;
            }

            free(curr);
            ht->elements--;
            return;
        }

        prev = curr;
        curr = curr->next;
    }
}

void ht_free(HashTable *ht) {
    for(int i = 0; i < ht->size; i++) {
        HashEntry *curr = ht->entries[i];
        while(curr != NULL) {
            HashEntry *next = curr->next;
            free(curr);

            curr = next;
        }
    }

    free(ht->entries);
    free(ht);
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
