#include <stdio.h>
#include <stdbool.h>
#include <ncurses.h>
#include <string.h>

#include "list.h"

#define INPUT_HEIGHT 3

/* 
 * 1.) Want to add commands to control interface: 
 *  -- need to sanitize input to avoid empty messages
 *  -- /disconnect --> disconnect from chat server
 *  -- /connect --> connect to the chat server (connected by default)
 *  -- /quit --> quit the application
 *
 * 2.) Want arrow keys and mouse/touchpad to scroll through chat history
 *  -- going to have to capture user input using wgetch
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
    box(inputWindow, 0, 0);
    wrefresh(inputWindow);

    // Chat Window
    WINDOW *chatWindow = newwin(chatHeight, width, 0, 0);
    scrollok(chatWindow, true);
    box(chatWindow, 0, 0);
    wrefresh(chatWindow);


    List *chatList = list_init();
    char message[MAX_MESSAGE_LENGTH];
    while(true) {
        // Update input window
        werase(inputWindow);
        box(inputWindow, 0, 0);
        mvwprintw(inputWindow, 1, 2, "Enter Message > ");
        wrefresh(inputWindow);

        // Get and sanitize input from user
        wgetnstr(inputWindow, message, MAX_MESSAGE_LENGTH - 1);
        message[MAX_MESSAGE_LENGTH - 1] = '\0';

        if(strncmp(message, "", strlen(message)) == 0) {
            continue;
        } else if (strncmp(message, "/quit", strlen("/quit")) == 0) {
            break;
        }

        // Update and print chat history
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

    // Cleanup everything
    list_free(chatList);
    delwin(inputWindow);
    endwin();

    return 0;
}





