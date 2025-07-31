#include "client.h"
#include <bits/pthreadtypes.h>
#include <fcntl.h>
#include <ncurses.h>
#include <pthread.h>


int min(int a, int b) {
    return a < b ? a : b;
}

pthread_mutex_t tuiMutex = PTHREAD_MUTEX_INITIALIZER;
void update_tui(WINDOW *inputWindow, WINDOW *chatWindow, List *chatList, GapBuffer *gapBuffer, int *cursorPosPtr, int *inputIndexPtr, int inputLength) {
    pthread_mutex_lock(&tuiMutex);
    // Update chat window
    werase(chatWindow);
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

    struct sockaddr_in recvAddr;
    socklen_t recvSize = sizeof(recvAddr);

    while(true) {
        char *message = calloc(MAX_MESSAGE_LENGTH + 1, sizeof(char));
        message[MAX_MESSAGE_LENGTH] = '\0';

        // Block until the server sends us a message
        recvfrom(socketFD, message, MAX_MESSAGE_LENGTH, 0, (struct sockaddr *)&recvAddr, &recvSize);

        pthread_mutex_lock(&tuiMutex);
        list_add(chatList, message);
        pthread_mutex_unlock(&tuiMutex);

        update_tui(inputWindow, chatWindow, chatList,gapBuffer, cursorPosPtr, inputIndexPtr, inputLength);
    }
}


// Can probably get rid of this in a little
void print_user_input(GapBuffer *gapBuffer, WINDOW *window, int cursorPos, int inputLength, int inputIndex, pthread_mutex_t *tuiMutex) {
    char *input = get_string(gapBuffer);

    pthread_mutex_lock(tuiMutex);
    werase(window);
    box(window, 0, 0);
    mvwprintw(window, CURSOR_START_ROW, 2, "Enter Message > %.*s", min(gapBuffer->strLen, inputLength), input + inputIndex);
    wmove(window, CURSOR_START_ROW, cursorPos);
    wrefresh(window);
    pthread_mutex_unlock(tuiMutex);

    free(input);
}

// Going to update this to handle packet types and lengths
void send_message_to_server(GapBuffer *gapBuffer, int socketFD, struct sockaddr_in *serverAddr) {
    char *message = get_string(gapBuffer);
    sendto(socketFD, message, strlen(message), 0, (struct sockaddr *)serverAddr, sizeof(*serverAddr));
}

void handle_enter_pressed(GapBuffer *gapBuffer, WINDOW *inputWindow, int *cursorPosPtr, int *inputIndexPtr, pthread_mutex_t *tuiMutex, int socketFD, struct sockaddr_in *serverAddr) {
    // Send the user's message to the server
    send_message_to_server(gapBuffer, socketFD, serverAddr);

    // Reset input window
    *inputIndexPtr = 0;
    *cursorPosPtr = CURSOR_START_COL;
    gap_buffer_reset(gapBuffer);

    pthread_mutex_lock(tuiMutex);
    werase(inputWindow);
    mvwprintw(inputWindow, CURSOR_START_ROW, 2, "Enter Message > ");
    box(inputWindow, 0, 0);
    wrefresh(inputWindow);
    pthread_mutex_unlock(tuiMutex);
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
    int width = userArg->width;

    int cursorMinCol = CURSOR_START_COL;
    int cursorMaxCol = width - 2;
    int inputLength = cursorMaxCol - cursorMinCol;

    // Going to move this outside of this function
    pthread_mutex_lock(&tuiMutex);
    int inputIndex = 0;
    int cursorPos = cursorMinCol;
    pthread_mutex_unlock(&tuiMutex);

    while(true) {
        pthread_mutex_lock(&tuiMutex);
        int charsInLeft = gapBuffer->gapStart;
        int charsInRight = gapBuffer->size - gapBuffer->gapEnd - 1;
        pthread_mutex_unlock(&tuiMutex);

        // Get and sanitize input from user
        wmove(inputWindow, CURSOR_START_ROW, cursorPos);
        int c = wgetch(inputWindow);

        switch(c) {
            // Handle user pressing the backspace
            case KEY_BACKSPACE:
                pthread_mutex_lock(&tuiMutex);
                if(cursorPos == CURSOR_START_COL) {
                    continue;
                }
                gap_buffer_delete(gapBuffer);
                if(gapBuffer->strLen > inputLength && inputIndex > 0) {
                    inputIndex--;
                } else {
                    cursorPos--;
                }
                pthread_mutex_unlock(&tuiMutex);
                update_tui(inputWindow, chatWindow, chatList, gapBuffer, cursorPosPtr, inputIndexPtr, inputLength);
                break;



            // Handle user pressing the left arrow key
            case KEY_LEFT:
                pthread_mutex_lock(&tuiMutex);
                if(cursorPos == cursorMinCol && inputIndex == 0) {
                    continue;
                }

                if(cursorPos == cursorMinCol) {
                    inputIndex--;
                } else {
                    cursorPos--;
                }
                move_gap_left(gapBuffer);
                pthread_mutex_unlock(&tuiMutex);
                update_tui(inputWindow, chatWindow, chatList, gapBuffer, cursorPosPtr, inputIndexPtr, inputLength);
                break;

            // Handle user pressing the right arrow key
            case KEY_RIGHT:
                pthread_mutex_lock(&tuiMutex);
                if(cursorPos - cursorMinCol == charsInLeft + charsInRight) {
                    continue;
                } 

                if(cursorPos == cursorMaxCol && gapBuffer->strLen - inputIndex > inputLength) {
                    inputIndex++;
                } else if(cursorPos < cursorMaxCol) {
                    cursorPos++;
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

                handle_enter_pressed(gapBuffer, inputWindow, &cursorPos, &inputIndex, tuiMutex, socketFD, serverAddr);
                break;

            // Handle user pressing escape key 
            case ESC_KEY_CODE:
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
                if(cursorPos == cursorMaxCol) {
                    inputIndex++;
                } else {
                    cursorPos++;
                }
                pthread_mutex_lock(&tuiMutex);
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

    // Initialize mutex to syncronize tui updates
    pthread_mutex_t tuiMutex;
    pthread_mutex_init(&tuiMutex, NULL);

    // Pack argument that gets passed to handle_user_input
    UserInArg *userArg = calloc(1, sizeof(UserInArg));
    userArg->socketFD = socketFD;
    userArg->inputWindow = inputWindow;
    userArg->tuiMutex = &tuiMutex;
    userArg->width = width;
    userArg->serverAddr = &serverAddr;

    // Pack argument that gets passed to handle_server_input
    ServerInArg *serverArg = calloc(1, sizeof(ServerInArg));
    serverArg->socketFD = socketFD;
    serverArg->chatWindow = chatWindow;
    serverArg->tuiMutex = &tuiMutex;

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

    free(userArg);
    free(serverArg);

    return 0;
}
