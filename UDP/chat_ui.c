#include <stdio.h>
#include <stdbool.h>
#include <ncurses.h>
#include <string.h>
#include <ctype.h>

#include "list.h"
#include "gap_buffer.h"

#define INPUT_HEIGHT 3
#define ENTER_KEY_CODE 10
#define CURSOR_START_ROW 1
#define CURSOR_START_COL 18

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

void print_user_input(GapBuffer *gapBuffer, WINDOW *window, int cursorPos) {
    char *input = get_string(gapBuffer);
    werase(window);
    box(window, 0, 0);
    mvwprintw(window, CURSOR_START_ROW, 2, "Enter Message > %s", input);
    wmove(window, CURSOR_START_ROW, cursorPos);
    wrefresh(window);

    free(input);
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

    // Input Window
    WINDOW *inputWindow = newwin(INPUT_HEIGHT, width, height - INPUT_HEIGHT, 0);
    keypad(inputWindow, true);
    scrollok(inputWindow, true);
    box(inputWindow, 0, 0);
    wrefresh(inputWindow);

    // Chat Window
    WINDOW *chatWindow = newwin(chatHeight, width, 0, 0);
    scrollok(chatWindow, true);
    box(chatWindow, 0, 0);
    wrefresh(chatWindow);





    int cursorPos = CURSOR_START_COL;
    List *chatList = list_init();
    GapBuffer *inputBuffer = gap_buffer_init();
    char message[MAX_MESSAGE_LENGTH];

    mvwprintw(inputWindow, CURSOR_START_ROW, 2, "Enter Message > ");
    box(inputWindow, 0, 0);
    wrefresh(inputWindow);

    while(true) {
        // Get and sanitize input from user
        wmove(inputWindow, CURSOR_START_ROW, cursorPos);
        int c = wgetch(inputWindow);

        if(isprint(c) != 0) {
            int charsInLeft = inputBuffer->gapStart;
            int charsInRight = inputBuffer->size - inputBuffer->gapEnd - 1;
            if(charsInLeft + charsInRight == MAX_MESSAGE_LENGTH) {
                continue;
            }

            gap_buffer_insert(inputBuffer, (char)c);
            print_user_input(inputBuffer, inputWindow, ++cursorPos);
        } else if (c == KEY_BACKSPACE || c == 127){
            if(cursorPos == CURSOR_START_COL) {
                continue;
            }

            gap_buffer_delete(inputBuffer);
            print_user_input(inputBuffer, inputWindow, --cursorPos);
        } else if (c == KEY_LEFT) {
            if(cursorPos == CURSOR_START_COL) {
                continue;
            }

            move_gap_left(inputBuffer);
            cursorPos--;
        } else if (c == KEY_RIGHT) {
            int charsInLeft = inputBuffer->gapStart;
            int charsInRight = inputBuffer->size - inputBuffer->gapEnd - 1;
            if(cursorPos == CURSOR_START_COL + charsInLeft + charsInRight) {
                continue;
            }

            move_gap_right(inputBuffer);
            cursorPos++;
        } else if (c == KEY_ENTER || c == ENTER_KEY_CODE) {
            // Update chat history window
            wmove(chatWindow, 1, 0);
            char *input = get_string(inputBuffer);
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
            gap_buffer_free(inputBuffer);
            inputBuffer = gap_buffer_init();

            cursorPos = CURSOR_START_COL;
            werase(inputWindow);
            mvwprintw(inputWindow, CURSOR_START_ROW, 2, "Enter Message > ");
            box(inputWindow, 0, 0);
            wrefresh(inputWindow);
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





