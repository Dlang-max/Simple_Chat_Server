#include "./headers/client.h"
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <unistd.h>

#define MAX_EVENTS 2

int min(int a, int b) {
    return a < b ? a : b;
}


void update_tui(UIContext *uiContext, NetworkContext *networkContext, AppContext *appContext) {
    /* Unpack UI context */
    WINDOW *inputWindow = uiContext->inputWindow;
    WINDOW *chatWindow = uiContext->chatWindow;
    int *cursorPosPtr = uiContext->cursorPosPtr;
    int *inputIndexPtr = uiContext->inputIndexPtr;
    int cursorMinCol = *(uiContext->cursorMinColPtr);
    int cursorMaxCol = uiContext->cursorMaxCol;
    int inputLength = *(uiContext->inputLengthPtr);

    // Unpack app context
    List *chatList = appContext->chatList;
    GapBuffer *gapBuffer = appContext->gapBuffer;
    bool *exitProgramPtr = appContext->exitProgramPtr;

    // Update chat window
    werase(chatWindow);
    wmove(chatWindow, 1, 0);
    ListNode *curr = chatList->head->next;
    while(curr != chatList->tail) {
        wprintw(chatWindow, " %s\n", curr->message);
        curr = curr->next;
    }
    box(chatWindow, 0, 0);
    wrefresh(chatWindow);

    // Update input window
    werase(inputWindow);

    char *input = get_string(gapBuffer);
    if(!*(networkContext->connectedPtr)) {
        mvwprintw(inputWindow, CURSOR_START_ROW, 2, "Enter Username > %.*s", min(gapBuffer->strLen, inputLength), input + *inputIndexPtr);
    } else {
        mvwprintw(inputWindow, CURSOR_START_ROW, 2, "Enter Message > %.*s", min(gapBuffer->strLen, inputLength), input + *inputIndexPtr);
    }

    wmove(inputWindow, CURSOR_START_ROW, *cursorPosPtr);
    box(inputWindow, 0, 0);
    wrefresh(inputWindow);

    free(input);
}








/* Methods for sending and parsing connect packets */
void send_connect_packet(int socketFD, struct sockaddr_in *serverAddr, GapBuffer *gapBuffer) {
    char *username = get_string(gapBuffer);
    int usernameLength = gapBuffer->strLen;
    uint8_t *packet = calloc(CONNECT_PACKET_BYTES, sizeof(uint8_t));

    // Pack packet type
    u_int8_t firstByte = 0;
    firstByte |= (uint8_t)((CONNECT_PACKET_ID & CLIENT_PACKET_TYPE_MASK) << 4);
    packet[0] = firstByte;

    // Pack sequence number (still need to add functionality)
    // packet[1] =

    // Pack username into packet
    memcpy(packet + CONNECT_HEADER_BYTES, username, usernameLength);

    // Send packet to server
    sendto(socketFD, packet, CONNECT_PACKET_BYTES, 0, (struct sockaddr *)serverAddr, sizeof(*serverAddr));

    // Need code here to ensure packet transmission was successful
    free(packet);
    free(username);
}

/* Methods for sending and parsing message packets */
char *parse_message_packet(uint8_t *packet) {
    int payloadLength = (packet[1] << 8) | packet[2];
    int usernameLength = strlen((char *)(packet + MESSAGE_PACKET_HEADER_BYTES));
    int messageLength = USERNAME_PREFIX_CHARS + usernameLength + USERNAME_SUFFIX_CHARS + payloadLength;
    char *message = calloc(messageLength + 1, sizeof(char));
    message[messageLength] = '\0';

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
void send_message_packet(int socketFD, struct sockaddr_in *serverAddr, AppContext *appContext) {
    GapBuffer *gapBuffer = appContext->gapBuffer;
    char *payload = get_string(gapBuffer);
    uint16_t payloadBytes = gapBuffer->strLen;

    char *username = appContext->username;
    int usernameBytes = strlen(username);

    int totalBytes = 1 + 2 + USERNAME_BYTES + payloadBytes;
    uint8_t *packet = calloc(totalBytes, sizeof(uint8_t));

    // Pack packet id
    packet[0] = (uint8_t)(MESSAGE_PACKET_ID);
    
    // Pack payload length
    packet[1] = (payloadBytes & 0xFF00) >> 8;
    packet[2] = payloadBytes & 0xFF;

    // Pack username
    memcpy(packet + 3, username, usernameBytes);

    // Pack payload
    memcpy(packet + 3 + USERNAME_BYTES, payload, payloadBytes);

    // Send packet to server
    sendto(socketFD, packet, totalBytes, 0, (struct sockaddr *)serverAddr, sizeof(*serverAddr));

    // Need code here to ensure packet transmission was successful
    free(packet);
    free(payload);
}


char *parse_received_packet(char *packet) {
    int packetType = packet[0];
    switch(packetType) {
        case DISCONNECT_PACKET_ID:
            break;

        case ACK_PACKET_ID:
            break;

        case MESSAGE_PACKET_ID:
            char *message = parse_message_packet(packet);
            return message; 
            break;
        default:
            break;
    }

    return NULL;
}


void handle_server_input(NetworkContext *networkContext, AppContext *appContext) {
    int socketFD = networkContext->socketFD;
    struct sockaddr_in recvAddr;
    socklen_t recvAddrSize = sizeof(recvAddr);

    char packet[MAX_MESSAGE_SIZE + 1];
    packet[MAX_MESSAGE_SIZE] = '\0';

    recvfrom(socketFD, packet, MAX_MESSAGE_LENGTH, 0, (struct sockaddr *)&recvAddr, &recvAddrSize);

    char *message = parse_received_packet(packet);
    if(packet != NULL) {
        list_add(appContext->chatList, message);
    }
}

// Going to update this to handle packet types and lengths
void send_packet_to_server(int socketFD, struct sockaddr_in *serverAddr, uint8_t packetType, AppContext *appContext) {
    switch(packetType) {
        case CONNECT_PACKET_ID:
            //send_connect_packet(socketFD, serverAddr, gapBuffer);
            break;

        case DISCONNECT_PACKET_ID:
            break;

        case MESSAGE_PACKET_ID:
            send_message_packet(socketFD, serverAddr, appContext);
            break;

        default:
            break;
    }

}

void handle_enter_pressed(UIContext *uiContext, NetworkContext *networkContext, AppContext *appContext) {
    bool *connectedPtr = networkContext->connectedPtr;
    GapBuffer *gapBuffer = appContext->gapBuffer;
    if(!*connectedPtr) {
        char *username = get_string(gapBuffer);
        appContext->username = username;
        *connectedPtr = true;
    } else {
        int socketFD = networkContext->socketFD;
        struct sockaddr_in *serverAddr = networkContext->serverAddr;
        send_packet_to_server(socketFD, serverAddr, MESSAGE_PACKET_ID, appContext);
    }

    // Reset input accumulators and gap buffer
    int *cursorPosPtr = uiContext->cursorPosPtr;
    int *inputIndexPtr = uiContext->inputIndexPtr;
    int *cursorMinColPtr = uiContext->cursorMinColPtr;
    int cursorMaxCol = uiContext->cursorMaxCol;
    int *inputLengthPtr = uiContext->inputLengthPtr;
    int *maxInputLengthPtr = uiContext->maxInputLengthPtr;
 
    bool connected = *connectedPtr;
    *cursorPosPtr = connected ? CURSOR_START_COL_MESSAGE : CURSOR_START_COL_USERNAME;
    *inputIndexPtr = 0;
    *cursorMinColPtr = *cursorPosPtr;
    *inputLengthPtr = cursorMaxCol - *cursorMinColPtr;
    *maxInputLengthPtr = connected ? MAX_MESSAGE_LENGTH : USERNAME_BYTES;
    gap_buffer_reset(gapBuffer);
}

void handle_user_input(UIContext *uiContext, NetworkContext *networkContext, AppContext *appContext) {
    /* Unpack UI context */
    WINDOW *inputWindow = uiContext->inputWindow;
    WINDOW *chatWindow = uiContext->chatWindow;
    int *cursorPosPtr = uiContext->cursorPosPtr;
    int *inputIndexPtr = uiContext->inputIndexPtr;
    int cursorMinCol = *(uiContext->cursorMinColPtr);
    int cursorMaxCol = uiContext->cursorMaxCol;
    int inputLength = *(uiContext->inputLengthPtr);
    int maxInputLength = *(uiContext->maxInputLengthPtr);

    // Unpack app context
    GapBuffer *gapBuffer = appContext->gapBuffer;
    bool *exitProgramPtr = appContext->exitProgramPtr;

    int charsInLeft = gapBuffer->gapStart;
    int charsInRight = gapBuffer->size - gapBuffer->gapEnd - 1;

    // Get and sanitize input from user
    wmove(inputWindow, CURSOR_START_ROW, *cursorPosPtr);
    int c = wgetch(inputWindow);
    switch(c) {
        // Handle user pressing the backspace
        case KEY_BACKSPACE:
            if(*cursorPosPtr == cursorMinCol) {
                break;
            }

            gap_buffer_delete(gapBuffer);
            if(gapBuffer->strLen >= inputLength && *inputIndexPtr > 0) {
                (*inputIndexPtr)--;
            } else {
                (*cursorPosPtr)--;
            }
            break;


        // Handle user pressing the left arrow key
        case KEY_LEFT:
            if(*cursorPosPtr == cursorMinCol && *inputIndexPtr == 0) {
                break;
            }

            if(*cursorPosPtr == cursorMinCol) {
                (*inputIndexPtr)--;
            } else {
                (*cursorPosPtr)--;
            }
            move_gap_left(gapBuffer);
            break;

        // Handle user pressing the right arrow key
        case KEY_RIGHT:
            if(*cursorPosPtr - cursorMinCol == charsInLeft + charsInRight) {
                break;
            } 

            if(*cursorPosPtr == cursorMaxCol && gapBuffer->strLen - *inputIndexPtr > inputLength) {
                (*inputIndexPtr)++;
            } else if(*cursorPosPtr < cursorMaxCol) {
                (*cursorPosPtr)++;
            }
            move_gap_right(gapBuffer);
            break;

        // Handle user pressing enter
        case ENTER_KEY_CODE:
            if(gapBuffer->strLen == 0) {
                break;
            }

            handle_enter_pressed(uiContext, networkContext, appContext);
            break;

        // Handle user pressing escape key 
        case ESC_KEY_CODE:
            *exitProgramPtr = true;
            break;

        // Handle user enter text
        default:
            if(isprint(c) == 0) {
                break;
            }

            if(charsInLeft + charsInRight == maxInputLength) {
                break;
            }

            gap_buffer_insert(gapBuffer, (char)c);
            if(*cursorPosPtr == cursorMaxCol) {
                (*inputIndexPtr)++;
            } else {
                (*cursorPosPtr)++;
            }
            break;
    }
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
    nodelay(inputWindow, true);
    mvwprintw(inputWindow, CURSOR_START_ROW, 2, "Enter Username > ");
    wrefresh(inputWindow);

    // Initialize Chat Window
    WINDOW *chatWindow = newwin(chatHeight, width, 0, 0);
    scrollok(chatWindow, true);
    box(chatWindow, 0, 0);
    wrefresh(chatWindow);


    // Get file descriptor for socket
    int socketFD = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
    if(socketFD == -1) {
        perror("Error getting socket file descriptor");
        return 1;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_ADDR, &(serverAddr.sin_addr));

    // Initialize epoll
    struct epoll_event eventSTDIN, eventSock, events[MAX_EVENTS];
    int epollFD = epoll_create1(0);
    if(epollFD == -1) {
        perror("Error getting epoll file descriptor");
        return 1;
    }

    // Add stdin to epoll events
    eventSTDIN.events = EPOLLIN;
    eventSTDIN.data.fd = STDIN_FILENO;
    if(epoll_ctl(epollFD, EPOLL_CTL_ADD, STDIN_FILENO, &eventSTDIN) == -1) {
        perror("Error adding stdin file descriptor to events");
        return 1;
    }

    // Add socket to epoll events
    eventSock.events = EPOLLIN;
    eventSock.data.fd = socketFD;
    if(epoll_ctl(epollFD, EPOLL_CTL_ADD, socketFD, &eventSock) == -1) {
        perror("Error adding socket file descriptor to events");
        return 1;
    }

    // Initialize chat list
    List *chatList = list_init();

    // Initialize gap buffer
    GapBuffer *gapBuffer = gap_buffer_init();

    // Initialize input buffer variables
    int cursorMinCol = CURSOR_START_COL_USERNAME;
    int cursorMaxCol = width - 2;
    int cursorPos = cursorMinCol;
    int inputIndex = 0;
    int inputLength = cursorMaxCol - cursorMinCol;
    int maxInputLength = USERNAME_BYTES;

    // Boolean for whether or not we've connected to server
    bool connected = false;
    // Boolean for whether or not we should exit
    bool exitProgram = false;
    // Sequence number for packet retransmission
    uint16_t currSequenceNum = 0;


    UIContext *uiContext = calloc(1, sizeof(UIContext));
    uiContext->inputWindow = inputWindow;
    uiContext->chatWindow = chatWindow;
    uiContext->cursorPosPtr = &cursorPos;
    uiContext->inputIndexPtr = &inputIndex;
    uiContext->cursorMinColPtr = &cursorMinCol;
    uiContext->cursorMaxCol = cursorMaxCol;
    uiContext->inputLengthPtr = &inputLength;
    uiContext->maxInputLengthPtr = &maxInputLength;

    NetworkContext *networkContext = calloc(1, sizeof(NetworkContext));
    networkContext->socketFD = socketFD;
    networkContext->serverAddr = &serverAddr;
    networkContext->connectedPtr = &connected;

    AppContext *appContext = calloc(1, sizeof(AppContext));
    appContext->chatList = chatList;
    appContext->gapBuffer = gapBuffer;
    appContext->exitProgramPtr = &exitProgram;

    while(!exitProgram) {

        int numReadyToRead = epoll_wait(epollFD, events, MAX_EVENTS, 10);
        for(int i = 0; i < numReadyToRead; i++) {
            int readableFD = events[i].data.fd;
            if(readableFD == STDIN_FILENO) {
                /* Get input from user */
                handle_user_input(uiContext, networkContext, appContext);
            } else if(readableFD == socketFD) {
                /* Get input from server */
                handle_server_input(networkContext, appContext);
            }
        }

        update_tui(uiContext, networkContext, appContext);
    }

    delwin(inputWindow);
    delwin(chatWindow);
    endwin();

    list_free(chatList);
    gap_buffer_free(gapBuffer);

    free(uiContext);
    free(networkContext);
    free(appContext->username);
    free(appContext);

    return 0;
}
