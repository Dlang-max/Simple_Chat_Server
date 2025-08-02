#include "client.h"
#include <bits/pthreadtypes.h>
#include <fcntl.h>
#include <ncurses.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <uchar.h>
/*
* Packet structure:
* |___|_________|_________..._____
*   |      |       v
*   |      v       packet data
*   v      9 bits for packet size
*   3 bits for packet version
*
*/


int min(int a, int b) {
    return a < b ? a : b;
}

pthread_mutex_t tuiMutex = PTHREAD_MUTEX_INITIALIZER;
void update_tui(WINDOW *inputWindow, WINDOW *chatWindow, List *chatList, GapBuffer *gapBuffer, int *cursorPosPtr, int *inputIndexPtr, int inputLength) {
    pthread_mutex_lock(&tuiMutex);
    // Update chat window
    werase(chatWindow);
    wmove(chatWindow, 1, 0);
    ListNode *curr = chatList->head->next;
    while(curr != chatList->tail) {
        wprintw(chatWindow, " > ip:time --- %s\n", curr->message);
        curr = curr->next;
    }
    box(chatWindow, 0, 0);
    wrefresh(chatWindow);

    // Update input window
    werase(inputWindow);

    char *input = get_string(gapBuffer);
    mvwprintw(inputWindow, CURSOR_START_ROW, 2, "Enter Message > %.*s", min(gapBuffer->strLen, inputLength), input + *inputIndexPtr);

    wmove(inputWindow, CURSOR_START_ROW, *cursorPosPtr);
    box(inputWindow, 0, 0);
    wrefresh(inputWindow);
    pthread_mutex_unlock(&tuiMutex);
}

void *handle_server_input(void *arg) {
    // Unpack argument passed to function
    ServerInArg *serverArg = (ServerInArg *)arg;
    int socketFD = serverArg->socketFD;
    WINDOW *inputWindow = serverArg->inputWindow;
    WINDOW *chatWindow = serverArg->chatWindow;
    List *chatList = serverArg->chatList;
    GapBuffer *gapBuffer = serverArg->gapBuffer;
    int *cursorPosPtr = serverArg->cursorPosPtr;
    int *inputIndexPtr = serverArg->cursorPosPtr;
    int inputLength = serverArg->inputLength;
    bool *exitProgramPtr = serverArg->exitProgramPtr;

    struct sockaddr_in recvAddr;
    socklen_t recvSize = sizeof(recvAddr);

    while(true) {
        pthread_mutex_lock(&tuiMutex);
        if(*exitProgramPtr) {
            pthread_mutex_unlock(&tuiMutex);
            return NULL;
        }

        pthread_mutex_unlock(&tuiMutex);

        char *message = calloc(MAX_MESSAGE_LENGTH + 1, sizeof(char));
        message[MAX_MESSAGE_LENGTH] = '\0';

        // Block until the server sends us a message
        // Going to have to set this to non-blocking
        recvfrom(socketFD, message, MAX_MESSAGE_LENGTH, 0, (struct sockaddr *)&recvAddr, &recvSize);

        pthread_mutex_lock(&tuiMutex);
        list_add(chatList, message);
        pthread_mutex_unlock(&tuiMutex);

        update_tui(inputWindow, chatWindow, chatList,gapBuffer, cursorPosPtr, inputIndexPtr, inputLength);
    }
}



// Going to update this to handle packet types and lengths
void send_message_to_server(int socketFD, struct sockaddr_in *serverAddr, uint8_t packetType, GapBuffer *gapBuffer) {
    char *payload = get_string(gapBuffer);
    int payloadBytes = gapBuffer->strLen;
    int payloadHeaderBytes = (PACKET_TYPE_BITS + PACKET_LEN_BITS) / 2;
    int totalBytes = payloadBytes + payloadHeaderBytes;
    uint8_t *packet = calloc(totalBytes, sizeof(uint8_t));

    // Pack packet type
    u_int8_t firstByte = 0;
    firstByte |= (uint8_t)((packetType & 0x0F) << 4);

    // Pack packet length
    firstByte |= (uint8_t)((payloadBytes & 0xF00) >> 8);
    packet[0] = firstByte;
    packet[1] = (uint8_t)(payloadBytes & 0xFF);

    // Pack packet message
    memcpy(packet + payloadHeaderBytes, payload, payloadBytes);

    // Send packet to server
    sendto(socketFD, packet, totalBytes, 0, (struct sockaddr *)serverAddr, sizeof(*serverAddr));

    free(packet);
    free(payload);
}

void handle_enter_pressed(int socketFD, struct sockaddr_in *serverAddr, WINDOW *inputWindow, WINDOW *chatWindow, List *chatList, GapBuffer *gapBuffer, int *cursorPosPtr, int *inputIndexPtr, int inputLength) {
    // Send the user's message to the server
    // 2 is a placeholder -- I want to implement packet types
    send_message_to_server(socketFD, serverAddr, 2, gapBuffer);

    // Reset input window
    pthread_mutex_lock(&tuiMutex);
    *inputIndexPtr = 0;
    *cursorPosPtr = CURSOR_START_COL;
    gap_buffer_reset(gapBuffer);
    pthread_mutex_unlock(&tuiMutex);
    update_tui(inputWindow, chatWindow, chatList, gapBuffer, cursorPosPtr, inputIndexPtr, inputLength);

}

void *handle_user_input(void *arg) {
    // Unpack argument passed to function
    UserInArg *userArg = (UserInArg *)arg;
    int socketFD = userArg->socketFD;
    struct sockaddr_in *serverAddr = userArg->serverAddr;
    WINDOW *inputWindow = userArg->inputWindow; 
    WINDOW *chatWindow = userArg->chatWindow;
    List *chatList = userArg->chatList;
    GapBuffer *gapBuffer = userArg->gapBuffer;
    int *cursorPosPtr = userArg->cursorPosPtr;
    int *inputIndexPtr = userArg->inputIndexPtr;
    int cursorMinCol = userArg->cursorMinCol;
    int cursorMaxCol = userArg->cursorMaxCol;
    int inputLength = userArg->inputLength;
    int width = userArg->width;
    bool *exitProgramPtr = userArg->exitProgramPtr;

    while(!(*exitProgramPtr)) {
        int charsInLeft = gapBuffer->gapStart;
        int charsInRight = gapBuffer->size - gapBuffer->gapEnd - 1;

        // Get and sanitize input from user
        wmove(inputWindow, CURSOR_START_ROW, *cursorPosPtr);
        int c = wgetch(inputWindow);

        switch(c) {
            // Handle user pressing the backspace
            case KEY_BACKSPACE:
                if(*cursorPosPtr == CURSOR_START_COL) {
                    continue;
                }

                pthread_mutex_lock(&tuiMutex);
                gap_buffer_delete(gapBuffer);
                if(gapBuffer->strLen >= inputLength && *inputIndexPtr > 0) {
                    (*inputIndexPtr)--;
                } else {
                    (*cursorPosPtr)--;
                }
                pthread_mutex_unlock(&tuiMutex);
                update_tui(inputWindow, chatWindow, chatList, gapBuffer, cursorPosPtr, inputIndexPtr, inputLength);
                break;


            // Handle user pressing the left arrow key
            case KEY_LEFT:
                if(*cursorPosPtr == cursorMinCol && *inputIndexPtr == 0) {
                    continue;
                }

                pthread_mutex_lock(&tuiMutex);
                if(*cursorPosPtr == cursorMinCol) {
                    (*inputIndexPtr)--;
                } else {
                    (*cursorPosPtr)--;
                }
                move_gap_left(gapBuffer);
                pthread_mutex_unlock(&tuiMutex);
                update_tui(inputWindow, chatWindow, chatList, gapBuffer, cursorPosPtr, inputIndexPtr, inputLength);
                break;

            // Handle user pressing the right arrow key
            case KEY_RIGHT:


                if(*cursorPosPtr - cursorMinCol == charsInLeft + charsInRight) {
                    continue;
                } 

                pthread_mutex_lock(&tuiMutex);
                if(*cursorPosPtr == cursorMaxCol && gapBuffer->strLen - *inputIndexPtr > inputLength) {
                    (*inputIndexPtr)++;
                } else if(*cursorPosPtr < cursorMaxCol) {
                    (*cursorPosPtr)++;
                }
                move_gap_right(gapBuffer);
                pthread_mutex_unlock(&tuiMutex);
                update_tui(inputWindow, chatWindow, chatList, gapBuffer, cursorPosPtr, inputIndexPtr, inputLength);
                break;

            // Handle user pressing enter
            case ENTER_KEY_CODE:
                if(gapBuffer->strLen == 0) {
                    continue;
                }

                handle_enter_pressed(socketFD, serverAddr, inputWindow, chatWindow, chatList, gapBuffer, cursorPosPtr, inputIndexPtr, inputLength);
                break;

            // Handle user pressing escape key 
            case ESC_KEY_CODE:
                pthread_mutex_lock(&tuiMutex);
                *exitProgramPtr = true;
                pthread_mutex_unlock(&tuiMutex);
                break;

            // Handle user enter text
            default:
                if(isprint(c) == 0) {
                    continue;
                }

                if(charsInLeft + charsInRight == MAX_MESSAGE_LENGTH) {
                    continue;
                }

                pthread_mutex_lock(&tuiMutex);
                gap_buffer_insert(gapBuffer, (char)c);
                if(*cursorPosPtr == cursorMaxCol) {
                    (*inputIndexPtr)++;
                } else {
                    (*cursorPosPtr)++;
                }
                pthread_mutex_unlock(&tuiMutex);
                update_tui(inputWindow, chatWindow, chatList, gapBuffer, cursorPosPtr, inputIndexPtr, inputLength);
                break;
        }
    }

    return NULL;
}

int main(void) {
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();

    // Get screen dimensions
    int height, width;
    getmaxyx(stdscr, height, width);
    int chatHeight = height - INPUT_HEIGHT;

    // Initialize User Input Window
    WINDOW *inputWindow = newwin(INPUT_HEIGHT, width, height - INPUT_HEIGHT, 0);
    box(inputWindow, 0, 0);
    keypad(inputWindow, true);
    mvwprintw(inputWindow, CURSOR_START_ROW, 2, "Enter Message > ");
    wrefresh(inputWindow);

    // Initialize Chat Window
    WINDOW *chatWindow = newwin(chatHeight, width, 0, 0);
    scrollok(chatWindow, true);
    box(chatWindow, 0, 0);
    wrefresh(chatWindow);


    // Get file descriptor for socket
    int socketFD = socket(AF_INET, SOCK_DGRAM, 0);
    if(socketFD == -1) {
        perror("Error getting socket file descriptor");
        return 1;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_ADDR, &(serverAddr.sin_addr));

    // Initialize chat list
    List *chatList = list_init();

    // Initialize gap buffer
    GapBuffer *gapBuffer = gap_buffer_init();

    // Initialize input buffer variables
    int cursorMinCol = CURSOR_START_COL;
    int cursorMaxCol = width - 2;
    int cursorPos = cursorMinCol;
    int inputIndex = 0;
    int inputLength = cursorMaxCol - cursorMinCol;

    // Boolean for whether or not we should exit
    bool exitProgram = false;

    // Pack argument that gets passed to handle_user_input
    UserInArg *userArg = calloc(1, sizeof(UserInArg));
    userArg->socketFD = socketFD;
    userArg->serverAddr = &serverAddr;
    userArg->inputWindow = inputWindow;
    userArg->chatWindow = chatWindow;
    userArg->chatList = chatList;
    userArg->gapBuffer = gapBuffer;
    userArg->cursorPosPtr = &cursorPos;
    userArg->inputIndexPtr = &inputIndex;
    userArg->cursorMinCol = cursorMinCol;
    userArg->cursorMaxCol = cursorMaxCol;
    userArg->inputLength = inputLength;
    userArg->width = width;
    userArg->exitProgramPtr = &exitProgram;

    // Pack argument that gets passed to handle_server_input
    ServerInArg *serverArg = calloc(1, sizeof(ServerInArg));
    serverArg->socketFD = socketFD;
    serverArg->inputWindow = inputWindow;
    serverArg->chatWindow = chatWindow;
    serverArg->chatList = chatList;
    serverArg->gapBuffer = gapBuffer;
    serverArg->cursorPosPtr = &cursorPos;
    serverArg->inputIndexPtr = &inputIndex;
    serverArg->inputLength = inputLength;
    serverArg->exitProgramPtr = &exitProgram;

    // Initialize pthreads for handling user and server input
    pthread_t userInThread, serverInThread;
    if(pthread_create(&userInThread, NULL, handle_user_input, (void *)userArg) != 0) {
        perror("Error creating pthread for handling user input");
        return 1;
    }

    if(pthread_create(&serverInThread, NULL, handle_server_input, (void *)serverArg) != 0) {
        perror("Error creating pthrad for handling server input");
        return 1;
    }

    pthread_join(userInThread, NULL);
    pthread_join(serverInThread, NULL);

    // Clean up and free everything before exiting
    pthread_mutex_destroy(&tuiMutex);
    delwin(inputWindow);
    delwin(chatWindow);
    endwin();

    list_free(chatList);
    gap_buffer_free(gapBuffer);

    free(userArg);
    free(serverArg);

    return 0;
}
