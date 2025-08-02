#include "./headers/server.h"

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

void send_message_to_clients(int socketFD, ClientList *clientList, char *packet, int payloadLength) {
    for(int i = 0; i < clientList->size; i++) {
        struct sockaddr_in clientAddr = clientList->clients[i].clientAddr;
        sendto(socketFD, packet, PACKET_HEADER_BYTES + payloadLength, 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
    }
}

int min(int a, int b) {
    return a < b ? a : b;
}

char *parse_message_packet(char *packet, int payloadLength) {
    int messageLength = min(MAX_MESSAGE_LENGTH, payloadLength);
    char *message = calloc(messageLength + 1, sizeof(char));
    message[messageLength] = '\0';

    memcpy(message, packet + PACKET_HEADER_BYTES, messageLength);

    return message;
}

char *parse_received_packet(char *packet, int payloadLength) {
    int packetType = (packet[0] & UNPACK_PACKET_TYPE_MASK) >> 4;
    switch(packetType) {
        case MESSAGE_PACKET_ID:
            char *message = parse_message_packet(packet, payloadLength);
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

        int payloadLength = (packet[0] & UNPACK_PAYLOAD_LEN_UPPER_NIBBLE_MASK) << 8;
        payloadLength |= packet[1];
        char *message = parse_received_packet(packet, payloadLength);

        char clientIP[INET_ADDRSTRLEN];
        clientIP[INET_ADDRSTRLEN - 1] = '\0';
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);

        printf("%s:%d > %s\n", clientIP, ntohs(clientAddr.sin_port), message);
        update_clients(clientList, clientAddr);

        // Send data to clients
        send_message_to_clients(socketFD, clientList, packet, payloadLength);
    }

    free(clientList);
    if(close(socketFD) == -1) {
        perror("Error closing socket");
        return 1;
    }

    return 0;
}
