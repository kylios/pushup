#ifndef LIST_H
#define LIST_H

#include <stddef.h>

#include "type.h"

/* The header for any list item. */
struct list_elem
{
    struct list_elem* prev;
    struct list_elem* next;
};

/* Structure that represents a list. */
struct list
{
    struct list_elem head;
    struct list_elem tail;
};

#define LIST_ENTRY(ELEM, TYPE, MEMBER)    \
    ((TYPE*) ((uint8*) &(ELEM)->next  \
        - offsetof(TYPE, MEMBER.next)))

#define LIST_ELEM_INITIALIZER   \
{   \
    NULL,   \
    NULL    \
}

/* Comparator function for list elements. 
   Should always return the equivalent of A - B such that the return 
   value will be:
   -positive: A > B
   -negative: A < B
   -zero: A == B
*/
typedef 
    int list_compare_func (struct list_elem* a, struct list_elem* b,
            void* aux);

typedef void list_print_func (struct list_elem* e);


void list_init (struct list*);
void list_push_back (struct list*, struct list_elem*);
void list_push_front (struct list*, struct list_elem*);
struct list_elem* list_pop_back (struct list*);
struct list_elem* list_pop_front (struct list*);
void list_insert_before (struct list_elem* elem, struct list_elem* data);
void list_insert_after (struct list_elem* elem, struct list_elem* data);
void list_remove (struct list_elem*);
struct list_elem* list_find (struct list*, struct list_elem*,
        list_compare_func*, void*);

struct list_elem* list_front (struct list*);
struct list_elem* list_back (struct list*);
struct list_elem* list_head (struct list*);
struct list_elem* list_tail (struct list*);
struct list_elem* list_begin (struct list*);
struct list_elem* list_end (struct list*);
struct list_elem* list_next (struct list_elem*);
struct list_elem* list_prev (struct list_elem*);
bool list_empty (struct list*);

void list_insert_ordered (struct list*, struct list_elem*, 
        list_compare_func*, void*);

void list_print (struct list*, list_print_func* func);

#endif // LIST_H

