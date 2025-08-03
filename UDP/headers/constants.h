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

/*
 * Connect packet structure
 *
 * |0001|____________________|
 *   |            |
 *   |            V
 *   V            20 bits for username
 *   4 bits for packet type
 */

/*
 * Disconnect packet structure
 *
 * |0010|
 *   |
 *   V
 *   4 bits for packet type
 */

/*
 * Acknowledgement packet structure
 *
 * |0011|
 *   |
 *   V
 *   4 bits for packet type
 */

/*
 * Message packet structure
 *
 * |0100|____________|________ .... _____|
 *   |        |             V
 *   |        V             payload data
 *   V        12 bits for payload length
 *   4 bits for packet type
 */

/* Established constants */
#define BITS_IN_BYTE 8

/* Networking constants */
#define PORT 6969
#define MAX_MESSAGE_LENGTH 256
#define SERVER_ADDR "192.168.50.20"

/* Structural information for packets */
#define PACKET_TYPE_BITS 4
#define PAYLOAD_LEN_BITS 12
#define PACKET_SEQ_BITS 16
#define USERNAME_BYTES 20

#define CONNECT_HEADER_BYTES (PACKET_TYPE_BITS + 4 + PACKET_SEQ_BITS + BITS_IN_BYTE - 1) / BITS_IN_BYTE
#define CONNECT_PACKET_BYTES CONNECT_HEADER_BYTES + USERNAME_BYTES

// Placeholder for now
#define MAX_PACKET_LENGTH 256
#define PACKET_HEADER_BYTES (PACKET_TYPE_BITS + PACKET_SEQ_BITS + BITS_IN_BYTE - 1) / BITS_IN_BYTE
#define MAX_MESSAGE_PACKET_LENGTH (PACKET_HEADER_BYTES + ((PAYLOAD_LEN_BITS) / BITS_IN_BYTE) + MAX_MESSAGE_LENGTH)

/* Unpacking constants */
#define UNPACK_PACKET_TYPE_MASK 0xF0
#define UNPACK_PAYLOAD_LEN_UPPER_NIBBLE_MASK 0x0F

/* Packet types */
#define CONNECT_PACKET_ID 1
#define DISCONNECT_PACKET_ID 2
#define ACK_PACKET_ID 3
#define MESSAGE_PACKET_ID 4
