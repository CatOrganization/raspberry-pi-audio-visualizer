#include <stdlib.h>
#include <stdio.h>
#include "linked_list.h"

// Adds the given data to the head of this linked list
void linked_list_add(LinkedList *list, void *data)
{
    LinkedListNode *prev_head = list->head;
    
    list->head = malloc(sizeof(LinkedListNode));
    list->head->data = data;
    list->head->prev = NULL;
    list->head->next = prev_head;

    if (prev_head != NULL)
    {
        prev_head->prev = list->head;
    }

    list->size++;
}

void linked_list_for_each(LinkedList *list, linked_list_action_func *func)
{
    LinkedListNode *current = list->head;
    int should_retain;
    
    while (current != NULL)
    {
        should_retain = func(current->data);

        if (!should_retain)
        {
            // Rearrange pointers so current is no longer in the list
            if (current->prev != NULL)
            {
                current->prev->next = current->next;
            }

            if (current->next != NULL)
            {
                current->next->prev = current->prev;
            }
            
            if (current == list->head)
            {
                list->head = current->next;
            }

            LinkedListNode *old_current = current;
            current = current->next;
            free(old_current);

            list->size--;
        }
        else
        {
            current = current->next;
        }
    }
}
