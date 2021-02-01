#ifndef LINKED_LIST_H
#define LINKED_LIST_H

// A function that runs on a linked list item. Return false to remove the item that's being acted on
typedef int (linked_list_action_func)(void *);

typedef struct LinkedListNode {
    void *data;

    struct LinkedListNode *next;
    struct LinkedListNode *prev;
} LinkedListNode;

typedef struct LinkedList {
    LinkedListNode *head;
    int size;
} LinkedList;

void linked_list_add(LinkedList *list, void *data);
void linked_list_for_each(LinkedList *list, linked_list_action_func *func);


#endif
