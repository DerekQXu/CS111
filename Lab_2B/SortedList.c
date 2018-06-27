/* NAME: Derek Xu
 * EMAIL: derekqxu@g.ucla.edu
 * ID: 704751588
 */

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "SortedList.h"

void SortedList_insert(SortedList_t *list, SortedListElement_t *element){
    // check NULL input
    if (list == NULL || element == NULL)
        return;

    SortedListElement_t *node = list->next;
    // find proper location to insert node
    while (node != list){
        if (strcmp(element->key, node->key) <= 0)
            break;
        node = node->next;
    }
    // linkage
    element->next = node;
    element->prev = node->prev;
    // yield option
    if (opt_yield & INSERT_YIELD)
        sched_yield();
    node->prev->next = element;
    node->prev = element;
}

int SortedList_delete( SortedListElement_t *element){
    // check NULL input
    if (element == NULL)
        return 2;

    //check for corruption
    if(element->next->prev != element->prev->next)
        return 1;

    // yield option
    if (opt_yield & DELETE_YIELD)
        sched_yield();

    // delete
    element->next->prev = element->prev;
    element->prev->next = element->next;
    return 0;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key){
    // check NULL input
    if (list == NULL || key == NULL)
        return NULL;

    // scanning for the node
    SortedListElement_t *node = list->next;
    while (node != list){
        if (strcmp(node->key, key) == 0)
            return node;
        // yield option
        if (opt_yield & LOOKUP_YIELD)
            sched_yield();
        node = node->next;
    }
    return NULL;
}

int SortedList_length(SortedList_t *list){
    // check NULL input
    if (list == NULL)
        return -1;

    // count the length
    int len = 0;
    SortedListElement_t *node = list->next;
    while (node != list){
        ++len;
        // yield option
        if (opt_yield & LOOKUP_YIELD)
            sched_yield();
        node = node->next;
    }
    return len;
}
