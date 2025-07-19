#include <stdio.h>
#include <stdbool.h>

#include <ncurses.h>

#define INPUT_HEIGHT 3
#define MAX_MESSAGE_LENGTH 512

int main(int argc, char *argv[]) {
    char message[MAX_MESSAGE_LENGTH];

    // Initialize ncurses
    initscr();
    cbreak();

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

    while(true) {
        // Update input window
        werase(inputWindow);
        box(inputWindow, 0, 0);
        mvwprintw(inputWindow, 1, 2, "Enter Message > ");
        wrefresh(inputWindow);

        // Get input from user
        wgetnstr(inputWindow, message, MAX_MESSAGE_LENGTH - 1);

        // Update chat history window
        werase(chatWindow);
        box(chatWindow, 0, 0);
        for(int i = 0; i < 40; i++) {
            wprintw(chatWindow, "This is kinda a long message message dkjafkldsajflkdjsa fkjadslk;fjdlaskjf lksadjflkasdj falsMessage %d\n", i);
        }
        // mvwprintw(chatWindow, 1, 2, "> %s", message);
        wrefresh(chatWindow);

    }

    delwin(inputWindow);
    endwin();


    return 0;
}





