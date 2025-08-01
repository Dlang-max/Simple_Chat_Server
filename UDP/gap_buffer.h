#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ncurses.h>

#define INIT_BUFFER_SIZE 10

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
typedef struct GapBuffer {
    int size;
    char *buffer;
    int gapStart;
    int gapEnd;
    int strLen;
} GapBuffer;


GapBuffer *gap_buffer_init();
void gap_buffer_free(GapBuffer *gapBuffer);
void gap_buffer_reset(GapBuffer *gapBuffer);
void move_gap_left(GapBuffer *gapBuffer);
void move_gap_right(GapBuffer *gapBuffer);
void gap_buffer_resize(GapBuffer *gapBuffer);
void gap_buffer_insert(GapBuffer *gapBuffer, char c);
void gap_buffer_delete(GapBuffer *gapBuffer);
char *get_string(GapBuffer *gapBuffer);
void gap_buffer_print(WINDOW *window, GapBuffer *gapBuffer);
