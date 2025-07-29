#include "gap_buffer.h"

void gap_buffer_print(GapBuffer *gapBuffer) {
    for(int i = 0; i < gapBuffer->size; i++) {
        char c = gapBuffer->buffer[i];
        if(c == '\0') {
            printf("_");
        } else {
            printf("%c", gapBuffer->buffer[i]);
        }
    }
    printf("\n");
}

int main(void) {
    GapBuffer *gapBuffer = gap_buffer_init();
    printf("Gap Buffer Size: %d\n", gapBuffer->size);
    gap_buffer_print(gapBuffer);


    gap_buffer_insert(gapBuffer, 'H');
    gap_buffer_insert(gapBuffer, 'E');
    gap_buffer_insert(gapBuffer, 'L');
    gap_buffer_insert(gapBuffer, 'L');
    gap_buffer_insert(gapBuffer, 'O');
    gap_buffer_print(gapBuffer);

    move_gap_right(gapBuffer);
    gap_buffer_print(gapBuffer);

    move_gap_left(gapBuffer);
    move_gap_left(gapBuffer);
    gap_buffer_print(gapBuffer);

    char *str = get_string(gapBuffer);
    printf("%s\n", str);
    free(str);

    move_gap_right(gapBuffer);
    gap_buffer_print(gapBuffer);
    gap_buffer_free(gapBuffer);

    gapBuffer = gap_buffer_init();
    for(int i = 0; i < 2; i++) {
        gap_buffer_insert(gapBuffer, (char)('a' + i));
    }

    move_gap_left(gapBuffer);
    move_gap_left(gapBuffer);
    gap_buffer_print(gapBuffer);

    for(int i = 2; i < 7; i++) {
        gap_buffer_insert(gapBuffer, (char)('a' + i));
    }
    gap_buffer_print(gapBuffer);

    gap_buffer_delete(gapBuffer);
    gap_buffer_delete(gapBuffer);
    gap_buffer_print(gapBuffer);

    printf("Gap Start: %d\n", gapBuffer->gapStart);
    printf("Gap End: %d\n", gapBuffer->gapEnd);

    str = get_string(gapBuffer);
    printf("%s\n", str);
    free(str);


    gap_buffer_free(gapBuffer);

    return 0;
}
