#include "list.h"
#include <string.h>

List *list_init() {
    ListNode *head = calloc(1, sizeof(ListNode));
    ListNode *tail = calloc(1, sizeof(ListNode));

    head->next = tail;
    tail->prev = head;

    List *list = calloc(1, sizeof(List));
    list->size = 0;
    list->head = head;
    list->tail = tail;

    return list;
}

void list_free(List *list) {
    ListNode *curr = list->head;
    while(curr != NULL) {
        ListNode *next = curr->next;
        free(curr->message);
        free(curr);
        curr = next;
    }

    free(list);
}

void list_add(List *list, char *message) {
    ListNode *prev = list->tail->prev;
    ListNode *next = list->tail;

    ListNode *newNode = calloc(1, sizeof(ListNode));
    newNode->message = message;

    prev->next = newNode;
    newNode->prev = prev;
    newNode->next = next;
    next->prev = newNode;
    list->size++;
}
