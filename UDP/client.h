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

#define PORT 6969
#define MAX_MESSAGE_SIZE 256
#define SERVER_ADDR "192.168.50.20"

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
    int inputLength;
    int width;
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
} ServerInArg;

void *handle_server_input(void *arg);
void *handle_user_input(void *arg);
