#include "gap_buffer.h"
#include <ncurses.h>

/*
* Gap Buffer API
* -- gap_buffer_init : initialize gap buffer
* -- gap_buffer_free : free gap buffer
* -- move_gap_left : moves gap left in buffer
* -- move_gap_right : moves gap right in buffer
* -- gap_buffer_insert : inserts a character into the buffer
* -- gap_buffer_delete : deletes a character from the buffer
* -- gab_buffer_resize : resizes the buffer when the gap is full
* -- get_string : returns a string made from characters in the buffer
*/
GapBuffer *gap_buffer_init() {
    char *buffer = calloc(INIT_BUFFER_SIZE, sizeof(char));

    GapBuffer *gapBuffer = calloc(1, sizeof(GapBuffer));
    gapBuffer->size = INIT_BUFFER_SIZE;
    gapBuffer->buffer = buffer;
    gapBuffer->gapStart = 0;
    gapBuffer->gapEnd = INIT_BUFFER_SIZE - 1;
    gapBuffer->strLen = 0;

    return gapBuffer;
}

void gap_buffer_free(GapBuffer *gapBuffer) {
    free(gapBuffer->buffer);
    free(gapBuffer);
}

void gap_buffer_reset(GapBuffer *gapBuffer) {
    char *buffer = gapBuffer->buffer;
    free(buffer);

    char *newBuffer = calloc(INIT_BUFFER_SIZE, sizeof(char));
    gapBuffer->size = INIT_BUFFER_SIZE;
    gapBuffer->buffer = newBuffer;
    gapBuffer->gapStart = 0;
    gapBuffer->gapEnd = INIT_BUFFER_SIZE - 1;
    gapBuffer->strLen = 0;
}

void move_gap_left(GapBuffer *gapBuffer) {
    if(gapBuffer->gapStart == 0) {
        return;
    }

    char *buffer = gapBuffer->buffer;
    buffer[gapBuffer->gapEnd] = buffer[gapBuffer->gapStart - 1];
    buffer[gapBuffer->gapStart - 1] = '\0';

    gapBuffer->gapStart--;
    gapBuffer->gapEnd--;
}

void move_gap_right(GapBuffer *gapBuffer) {
    if(gapBuffer->gapEnd == gapBuffer->size - 1) {
        return;
    }

    char *buffer = gapBuffer->buffer;
    buffer[gapBuffer->gapStart] = buffer[gapBuffer->gapEnd + 1];
    buffer[gapBuffer->gapEnd + 1] = '\0';

    gapBuffer->gapStart++;
    gapBuffer->gapEnd++;
}

void gap_buffer_resize(GapBuffer *gapBuffer) {
    int newSize = 2 * gapBuffer->size;
    char *newBuffer = calloc(newSize, sizeof(char));

    int charsInLeft = gapBuffer->gapStart;
    int charsInRight = gapBuffer->size - gapBuffer->gapEnd - 1;

    memcpy(newBuffer, gapBuffer->buffer, charsInLeft);
    memcpy(newBuffer + newSize - charsInRight, gapBuffer->buffer + gapBuffer->gapEnd + 1, charsInRight); 

    gapBuffer->gapEnd = newSize - charsInRight - 1;
    gapBuffer->size = newSize;

    free(gapBuffer->buffer);
    gapBuffer->buffer = newBuffer;
}

void gap_buffer_insert(GapBuffer *gapBuffer, char c) {
    if(gapBuffer->gapStart == gapBuffer->gapEnd) {
        gap_buffer_resize(gapBuffer);
    }

    gapBuffer->buffer[gapBuffer->gapStart++] = c;
    gapBuffer->strLen++;
}

void gap_buffer_delete(GapBuffer *gapBuffer) {
    if(gapBuffer->gapStart == 0) {
        return;
    }

    gapBuffer->buffer[--gapBuffer->gapStart] = '\0';
    gapBuffer->strLen--;
}

char *get_string(GapBuffer *gapBuffer) {
    int charsInLeft = gapBuffer->gapStart;
    int charsInRight = gapBuffer->size - gapBuffer->gapEnd - 1;

    int numCharacters = charsInLeft + charsInRight;
    char *string = calloc(numCharacters + 1, sizeof(char));
    string[numCharacters] = '\0';

    memcpy(string, gapBuffer->buffer, charsInLeft);
    memcpy(string + charsInLeft, gapBuffer->buffer + gapBuffer->size - charsInRight, charsInRight);

    return string;
}

void gap_buffer_print(WINDOW *window, GapBuffer *gapBuffer) {
    for(int i = 0; i < gapBuffer->size; i++) {
        if(gapBuffer->buffer[i] == '\0') {
            wprintw(window, "_");
        } else {
            wprintw(window, "%c", gapBuffer->buffer[i]);
        }
    }
    wprintw(window, "\n");
    wrefresh(window);

}
