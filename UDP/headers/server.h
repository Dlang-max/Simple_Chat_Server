#include "constants.h"
#include "list.h"
#include "gap_buffer.h"

#define CLIENT_LIST_SIZE 5

/* Server structs */
typedef struct ClientListNode {
    struct sockaddr_in clientAddr;
} ClientListNode;


typedef struct ClientList {
    int size;
    ClientListNode *clients;
} ClientList;
