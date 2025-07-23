#include <stdlib.h>

#define MAX_MESSAGE_LENGTH 512

typedef struct ListNode {
    struct ListNode *next;
    struct ListNode *prev;

    char message[MAX_MESSAGE_LENGTH];
} ListNode;

typedef struct List {
    int size;
    struct ListNode *head;
    struct ListNode *tail;
} List;

List *list_init();
void list_free(List *list);
void list_add(List *list, char *message);

