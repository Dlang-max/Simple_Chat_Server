#include "constants.h"

typedef struct ListNode {
    struct ListNode *next;
    struct ListNode *prev;

    char *message;
} ListNode;

typedef struct List {
    int size;
    struct ListNode *head;
    struct ListNode *tail;
} List;

List *list_init();
void list_free(List *list);
void list_add(List *list, char *message);

