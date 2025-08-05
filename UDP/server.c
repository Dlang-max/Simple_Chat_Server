#include "./headers/server.h"
#include <sys/types.h>

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

void send_message_to_clients(int socketFD, ClientList *clientList, char *packet) {
    int payloadLength = (packet[1] << 8) | packet[2];
    for(int i = 0; i < clientList->size; i++) {
        struct sockaddr_in clientAddr = clientList->clients[i].clientAddr;
        sendto(socketFD, packet, 1 + 2 + USERNAME_BYTES + payloadLength, 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
    }
}

int min(int a, int b) {
    return a < b ? a : b;
}

int get_username_length(uint8_t *packet) {
    int length = 0;
    for(int i = 0; i < USERNAME_BYTES; i++) {
        if(packet[i] == '\0') {
            break;
        }

        length++;
    }

    return length;
}

char *parse_message_packet(uint8_t *packet) {
    int payloadLength = (packet[1] << 8) | packet[2];
    int usernameLength = get_username_length(packet + MESSAGE_PACKET_HEADER_BYTES); 
    int messageLength = USERNAME_PREFIX_CHARS + usernameLength + USERNAME_SUFFIX_CHARS + payloadLength;
    char *message = calloc(messageLength + 1, sizeof(char));
    message[messageLength + 1] = '\0';

    message[0] = '>';
    message[1] = ' ';
    memcpy(message + 2, packet + MESSAGE_PACKET_HEADER_BYTES, usernameLength);
    message[USERNAME_PREFIX_CHARS + usernameLength] = ' ';
    message[USERNAME_PREFIX_CHARS + usernameLength + 1] = '-';
    message[USERNAME_PREFIX_CHARS + usernameLength + 2] = '-';
    message[USERNAME_PREFIX_CHARS + usernameLength + 3] = '-';
    message[USERNAME_PREFIX_CHARS + usernameLength + 4] = ' ';
    memcpy(message + USERNAME_PREFIX_CHARS + usernameLength + USERNAME_SUFFIX_CHARS, packet + MESSAGE_PACKET_HEADER_BYTES + USERNAME_BYTES, payloadLength);

    return message;
}


char *parse_received_packet(char *packet) {
    int packetType = packet[0];
    switch(packetType) {
        case MESSAGE_PACKET_ID:
            char *message = parse_message_packet(packet);
            return message;
            break;
        default:
            break;

    }

    return NULL;
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
    socklen_t clientAddrSize = sizeof(clientAddr);

    ClientList *clientList = client_list_init();
    printf("Running Server...\n");
    while(true) {
        char packet[MAX_PACKET_LENGTH];
        // Receive UDP packets from clients
        recvfrom(socketFD, packet, MAX_PACKET_LENGTH, 0, (struct sockaddr *)&clientAddr, &clientAddrSize);

        char *message = parse_received_packet(packet);

        char clientIP[INET_ADDRSTRLEN];
        clientIP[INET_ADDRSTRLEN - 1] = '\0';
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);

        printf("%s:%d %s\n", clientIP, ntohs(clientAddr.sin_port), message);
        free(message);
        update_clients(clientList, clientAddr);

        // Send data to clients
        send_message_to_clients(socketFD, clientList, packet);
    }

    free(clientList);
    if(close(socketFD) == -1) {
        perror("Error closing socket");
        return 1;
    }

    return 0;
}
