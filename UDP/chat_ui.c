#include <stdio.h>
#include <stdbool.h>

#include <ncurses.h>

#define MAX_MESSAGE_LENGTH 512

int main(int argc, char *argv[]) {
    char message[MAX_MESSAGE_LENGTH];

    // Initialize ncurses
    initscr();
    cbreak();
    noecho();

    int height, width;
    getmaxyx(stdscr, height, width);

    int inputHeight = 3;
    int chatHeight = height - inputHeight;

    // Chat window
    WINDOW *chatWindow = newwin(inputHeight, width, height - 3, 0);
    box(chatWindow, 0, 0);

    while(true) {
        box(chatWindow, 0, 0);
        mvwprintw(chatWindow, 1, 2, "Enter Message> ");
        wrefresh(chatWindow);

        // Get input from user
        wgetnstr(chatWindow, message, MAX_MESSAGE_LENGTH);
    }

    delwin(chatWindow);
    endwin();


    return 0;
}





