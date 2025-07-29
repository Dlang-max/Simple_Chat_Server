#include <stdio.h>
#include <stdbool.h>
#include <ncurses.h>
#include <string.h>
#include <ctype.h>

#include "list.h"
#include "gap_buffer.h"

#define INPUT_HEIGHT 3
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

int main(int argc, char *argv[]) {
    // Initialize ncurses
    initscr();
    cbreak();

    // Get screen dimensions
    int height, width;
    getmaxyx(stdscr, height, width);
    int chatHeight = height - INPUT_HEIGHT;

    // Input Window
    WINDOW *inputWindow = newwin(INPUT_HEIGHT, width, height - INPUT_HEIGHT, 0);
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

    mvwprintw(inputWindow, 1, 2, "Enter Message > ");
    box(inputWindow, 0, 0);
    wrefresh(inputWindow);

    while(true) {
        // Get and sanitize input from user
        wmove(inputWindow, 1, cursorPos);
        int c = wgetch(inputWindow);

        if(isprint(c)) {
            gap_buffer_insert(inputBuffer, (char)c);
            char *input = get_string(inputBuffer);
            fprintf(stdout, "%s\n", input);
            werase(inputWindow);
            mvwprintw(inputWindow, 1, 2, "Enter Message > %s", input);
            box(inputWindow, 0, 0);
            wrefresh(inputWindow);
            free(input);

        } else if (c == KEY_ENTER) {
            werase(chatWindow);
            wmove(chatWindow, 1, 0);
            list_add(chatList, message);
            ListNode *curr = chatList->head->next;
            while(curr != chatList->tail) {
                wprintw(chatWindow, " > ip:time --- %s\n", curr->message);
                curr = curr->next;
            }
            box(chatWindow, 0, 0);
            wrefresh(chatWindow);

            
        }
















        //wgetnstr(inputWindow, message, MAX_MESSAGE_LENGTH - 1);
        //message[MAX_MESSAGE_LENGTH - 1] = '\0';
/*
        if(strncmp(message, "", strlen(message)) == 0) {
            continue;
        } else if (strncmp(message, "/quit", strlen("/quit")) == 0) {
            break;
        }
*/
        // Update and print chat history
    }

    // Cleanup everything
    list_free(chatList);
    delwin(inputWindow);
    endwin();

    return 0;
}





