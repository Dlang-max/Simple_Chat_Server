#include "client.h"
#include <bits/pthreadtypes.h>
#include <ncurses.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/socket.h>

void *handle_server_input(void *arg) {
    // Unpack argument passed to function
    ServerInArg *serverArg = (ServerInArg *)arg;
    int socketFD = serverArg->socketFD;
    WINDOW *chatWindow = serverArg->chatWindow;
    pthread_mutex_t *tuiMutex = serverArg->tuiMutex;

    struct sockaddr_in recvAddr;
    socklen_t recvSize;

    List *chatList = list_init();
    while(true) {
        char *message = calloc(MAX_MESSAGE_LENGTH + 1, sizeof(char));
        message[MAX_MESSAGE_LENGTH] = '\0';

        // Block until the server sends us a message
        recvfrom(socketFD, message, MAX_MESSAGE_LENGTH, 0, (struct sockaddr *)&recvAddr, &recvSize);

        list_add(chatList, message);
        free(message);

        ListNode *curr = chatList->head->next;
        while(curr != chatList->tail) {
            pthread_mutex_lock(tuiMutex);
            wprintw(chatWindow, " > ip:time --- %s\n", curr->message);
            pthread_mutex_unlock(tuiMutex);
            curr = curr->next;
        }
        wrefresh(chatWindow);
    }
}

int min(int a, int b) {
    return a < b ? a : b;
}

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

void send_message_to_server(GapBuffer *gapBuffer, int socketFD, struct sockaddr_in *serverAddr) {
    char *message = get_string(gapBuffer);
    sendto(socketFD, message, strlen(message), 0, (struct sockaddr *)serverAddr, sizeof(*serverAddr));
}

void handle_enter_pressed(GapBuffer *gapBuffer, WINDOW *inputWindow, int *cursorPosPtr, int *inputIndexPtr, int socketFD, struct sockaddr_in *serverAddr) {
    // Send the user's message to the server
    send_message_to_server(gapBuffer, socketFD, serverAddr);

    // Reset input window
    *inputIndexPtr = 0;
    *cursorPosPtr = CURSOR_START_COL;
    gap_buffer_reset(gapBuffer);

    werase(inputWindow);
    mvwprintw(inputWindow, CURSOR_START_ROW, 2, "Enter Message > ");
    box(inputWindow, 0, 0);
    wrefresh(inputWindow);
}

void *handle_user_input(void *arg) {
    // Unpack argument passed to function
    UserInArg *userArg = (UserInArg *)arg;
    int socketFD = userArg->socketFD;
    WINDOW *inputWindow = userArg->inputWindow; 
    pthread_mutex_t *tuiMutex = userArg->tuiMutex;
    int width = userArg->width;
    struct sockaddr_in *serverAddr = userArg->serverAddr;

    int cursorMinCol = CURSOR_START_COL;
    int cursorMaxCol = width - 2;
    int inputLength = cursorMaxCol - cursorMinCol;

    int inputIndex = 0;
    int cursorPos = CURSOR_START_COL;
    GapBuffer *inputBuffer = gap_buffer_init();
    while(true) {
        int charsInLeft = inputBuffer->gapStart;
        int charsInRight = inputBuffer->size - inputBuffer->gapEnd - 1;

        // Get and sanitize input from user
        pthread_mutex_lock(tuiMutex);
        wmove(inputWindow, CURSOR_START_ROW, cursorPos);
        int c = wgetch(inputWindow);
        pthread_mutex_unlock(tuiMutex);

        switch(c) {
            // Handle user pressing the backspace
            case KEY_BACKSPACE:
                if(cursorPos == CURSOR_START_COL) {
                    continue;
                }
                gap_buffer_delete(inputBuffer);
                if(inputBuffer->strLen > inputLength && inputIndex > 0) {
                    inputIndex--;
                } else {
                    cursorPos--;
                }
                print_user_input(inputBuffer, inputWindow, cursorPos, inputLength, inputIndex, tuiMutex);
                break;

            // Handle user pressing the left arrow key
            case KEY_LEFT:
                if(cursorPos == cursorMinCol && inputIndex == 0) {
                    continue;
                }

                if(cursorPos == cursorMinCol) {
                    inputIndex--;
                } else {
                    cursorPos--;
                }
                move_gap_left(inputBuffer);
                print_user_input(inputBuffer, inputWindow, cursorPos, inputLength, inputIndex, tuiMutex);
                break;

            // Handle user pressing the right arrow key
            case KEY_RIGHT:
                if(cursorPos - cursorMinCol == charsInLeft + charsInRight) {
                    continue;
                } 

                if(cursorPos == cursorMaxCol && inputBuffer->strLen - inputIndex > inputLength) {
                    inputIndex++;
                } else if(cursorPos < cursorMaxCol) {
                    cursorPos++;
                }
                move_gap_right(inputBuffer);
                print_user_input(inputBuffer, inputWindow, cursorPos, inputLength, inputIndex, tuiMutex);
                break;

            // Handle user pressing enter
            case ENTER_KEY_CODE:
                if(inputBuffer->strLen == 0) {
                    continue;
                }

                handle_enter_pressed(inputBuffer, inputWindow, &cursorPos, &inputIndex, socketFD, serverAddr);
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

                gap_buffer_insert(inputBuffer, (char)c);
                if(cursorPos == cursorMaxCol) {
                    inputIndex++;
                } else {
                    cursorPos++;
                }

                print_user_input(inputBuffer, inputWindow, cursorPos, inputLength, inputIndex, tuiMutex);
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
