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

#include "list.h"
#include "gap_buffer.h"

#define BITS_IN_BYTE 8

#define PORT 6969
#define MAX_MESSAGE_SIZE 256
#define SERVER_ADDR "192.168.50.20"

#define PACKET_TYPE_BITS 4
#define PACKET_LEN_BITS 12
#define PACKET_HEADER_BYTES (PACKET_TYPE_BITS + PACKET_LEN_BITS + BITS_IN_BYTE - 1) / BITS_IN_BYTE 
#define MAX_PACKET_SIZE (PACKET_HEADER_BYTES + MAX_MESSAGE_SIZE)
#define CLIENT_PACKET_TYPE_MASK 0x0F
#define CLIENT_PAYLOAD_LEN_UPPER_NIBBLE_MASK 0xF00
#define CLIENT_PAYLOAD_LEN_LOWER_BYTE_MASK 0xFF

#define SERVER_PACKET_TYPE_MASK 0xF0
#define SERVER_PAYLOAD_LEN_UPPER_NIBBLE_MASK 0x0F

/* Packet types */
#define MESSAGE_PACKET_ID 4

#define INPUT_HEIGHT 3
#define CURSOR_START_ROW 1
#define CURSOR_START_COL 18

#define ESC_KEY_CODE 27
#define ENTER_KEY_CODE 10
#define BACKSPACE_KEY_CODE 127

typedef struct UserInArg {
    int socketFD;
    struct sockaddr_in *serverAddr;
    WINDOW *inputWindow;
    WINDOW *chatWindow;
    List *chatList;
    GapBuffer *gapBuffer;
    int *cursorPosPtr;
    int *inputIndexPtr;
    int cursorMinCol;
    int cursorMaxCol;
    int inputLength;
    int width;
    bool *connected;
    bool *exitProgramPtr;
} UserInArg;

typedef struct ServerInArg {
    int socketFD;
    WINDOW *inputWindow;
    WINDOW *chatWindow;
    List *chatList;
    GapBuffer *gapBuffer;
    int *cursorPosPtr;
    int *inputIndexPtr;
    int inputLength;
    bool *connected;
    bool *exitProgramPtr;
} ServerInArg;

void *handle_server_input(void *arg);
void *handle_user_input(void *arg);
