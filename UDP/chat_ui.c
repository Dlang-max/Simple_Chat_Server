#include <stdbool.h>
#include <ncurses.h>
#include <stdlib.h>
#include <ctype.h>

#include "list.h"
#include "gap_buffer.h"

#define INPUT_HEIGHT 3
#define CURSOR_START_ROW 1
#define CURSOR_START_COL 18

#define ESC_KEY_CODE 27
#define ENTER_KEY_CODE 10
#define BACKSPACE_KEY_CODE 127

/* 
 * 1.) Want to add commands to control interface: 
 *  -- need to sanitize input to avoid empty messages
 *  -- /disconnect --> disconnect from chat server
 *  -- /connect --> connect to the chat server (connected by default)
 *  -- /quit --> quit the application
 *
 * 2.) Want arrow keys and mouse/touchpad to scroll through chat history
 *  -- going to have to capture user input using wgetch
 *  * <-- and --> move cursor
 *  * ^ and v move chat history up and down
 *
 *  3.)
 *  -- going to implement a gap buffer to handle user input
 */

int min(int a, int b) {
    return a < b ? a : b;
}

void print_user_input(GapBuffer *gapBuffer, WINDOW *window, int cursorPos, int inputLength, int inputIndex) {
    char *input = get_string(gapBuffer);
    werase(window);
    box(window, 0, 0);
    mvwprintw(window, CURSOR_START_ROW, 2, "Enter Message > %.*s", min(gapBuffer->strLen, inputLength), input + inputIndex);
    wmove(window, CURSOR_START_ROW, cursorPos);
    wrefresh(window);

    free(input);
}

void handle_enter_pressed(GapBuffer *gapBuffer, List *chatList, WINDOW *inputWindow, WINDOW *chatWindow, int *cursorPosPtr, int *inputIndexPtr) {
    wmove(chatWindow, 1, 0);
    char *input = get_string(gapBuffer);
    list_add(chatList, input);
    ListNode *curr = chatList->head->next;
    while(curr != chatList->tail) {
        wprintw(chatWindow, " > ip:time --- %s\n", curr->message);
        curr = curr->next;
    }

    box(chatWindow, 0, 0);
    wrefresh(chatWindow);
    free(input);

    // Reset input window
    gap_buffer_reset(gapBuffer);
    *inputIndexPtr = 0;

    *cursorPosPtr = CURSOR_START_COL;
    werase(inputWindow);
    mvwprintw(inputWindow, CURSOR_START_ROW, 2, "Enter Message > ");
    box(inputWindow, 0, 0);
    wrefresh(inputWindow);
}

int main(int argc, char *argv[]) {
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();

    // Get screen dimensions
    int height, width;
    getmaxyx(stdscr, height, width);
    int chatHeight = height - INPUT_HEIGHT;
    int cursorMinCol = CURSOR_START_COL;
    int cursorMaxCol = width - 2;
    int inputLength = cursorMaxCol - cursorMinCol;

    // Input Window
    WINDOW *inputWindow = newwin(INPUT_HEIGHT, width, height - INPUT_HEIGHT, 0);
    keypad(inputWindow, true);
    box(inputWindow, 0, 0);
    wrefresh(inputWindow);

    // Chat Window
    WINDOW *chatWindow = newwin(chatHeight, width, 0, 0);
    scrollok(chatWindow, true);
    box(chatWindow, 0, 0);
    wrefresh(chatWindow);

    int inputIndex = 0;
    int cursorPos = CURSOR_START_COL;
    List *chatList = list_init();
    GapBuffer *inputBuffer = gap_buffer_init();

    mvwprintw(inputWindow, CURSOR_START_ROW, 2, "Enter Message > ");
    box(inputWindow, 0, 0);
    wrefresh(inputWindow);

    int chatOffset = 0;
    bool exitProgram = false;
    while(!exitProgram) {
        int charsInLeft = inputBuffer->gapStart;
        int charsInRight = inputBuffer->size - inputBuffer->gapEnd - 1;

        // Get and sanitize input from user
        wmove(inputWindow, CURSOR_START_ROW, cursorPos);
        int c = wgetch(inputWindow);

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
                print_user_input(inputBuffer, inputWindow, cursorPos, inputLength, inputIndex);
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
                print_user_input(inputBuffer, inputWindow, cursorPos, inputLength, inputIndex);
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
                print_user_input(inputBuffer, inputWindow, cursorPos, inputLength, inputIndex);
                break;

            // Handle user pressing enter
            case ENTER_KEY_CODE:
                if(inputBuffer->strLen == 0) {
                    continue;
                }

                handle_enter_pressed(inputBuffer, chatList, inputWindow, chatWindow, &cursorPos, &inputIndex);
                break;

            // Handle user pressing escape key 
            case ESC_KEY_CODE:
                exitProgram = true;
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

                print_user_input(inputBuffer, inputWindow, cursorPos, inputLength, inputIndex);
                break;
        }
    }

    // Cleanup everything
    list_free(chatList);
    gap_buffer_free(inputBuffer);
    delwin(inputWindow);
    delwin(chatWindow);
    endwin();

    return 0;
}
