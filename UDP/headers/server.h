#include "constants.h"
#include "list.h"
#include "gap_buffer.h"

#define CLIENT_LIST_SIZE 5

/* Unpacking constants */
#define SERVER_PACKET_TYPE_MASK 0xF0
#define SERVER_PAYLOAD_LEN_UPPER_NIBBLE_MASK 0x0F

/* Server structs */
typedef struct ClientListNode {
    struct sockaddr_in clientAddr;
} ClientListNode;


typedef struct ClientList {
    int size;
    ClientListNode *clients;
} ClientList;
