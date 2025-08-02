#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <bits/pthreadtypes.h>
#include <ncurses.h>

/* Established constants */
#define BITS_IN_BYTE 8

/* Networking constants */
#define PORT 6969
#define MAX_MESSAGE_LENGTH 256
#define SERVER_ADDR "192.168.50.20"

/* Structural information for packets */
#define PACKET_TYPE_BITS 4
#define PACKET_LEN_BITS 12
#define PACKET_HEADER_BYTES (PACKET_TYPE_BITS + PACKET_LEN_BITS + BITS_IN_BYTE - 1) / BITS_IN_BYTE
#define MAX_PACKET_LENGTH (PACKET_HEADER_BYTES + MAX_MESSAGE_LENGTH)

/* Unpacking constants */
#define UNPACK_PACKET_TYPE_MASK 0xF0
#define UNPACK_PAYLOAD_LEN_UPPER_NIBBLE_MASK 0x0F

/* Packet types */
#define MESSAGE_PACKET_ID 4
