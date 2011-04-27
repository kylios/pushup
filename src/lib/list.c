#include <stddef.h>

#include "lib/list.h"
#include "type.h"
#include "debug.h"

void 
list_init (struct list* list)
{
    if (list == NULL)   return;

    list->head.prev = NULL;
    list->head.next = &list->tail;
    list->tail.prev = &list->head;
    list->tail.next = NULL;
};

void 
list_push_back (struct list* list, struct list_elem* e)
{
    if (list == NULL)   return;

    list_insert_before (&list->tail, e);    
};

void 
list_push_front (struct list* list, struct list_elem* e)
{
    if (list == NULL)   return;    

    list_insert_after (&list->head, e);
};

struct list_elem* 
list_pop_back (struct list* list)
{
    if (list == NULL)   return;    

    struct list_elem* e = list_back (list);
    list_remove (e);
    return e;
};

struct list_elem* 
list_pop_front (struct list* list)
{
    if (list == NULL)   return;

    struct list_elem* e = list_front (list);
    list_remove (e);
    return e;
};

void 
list_insert_before (struct list_elem* elem, struct list_elem* data)
{
    if (elem == NULL || data == NULL)   return;
    ASSERT (elem->prev != NULL);
    
    struct list_elem* prev = elem->prev;    
    data->prev = prev;
    data->next = elem;
    elem->prev = data;
    prev->next = data;
};

void 
list_insert_after (struct list_elem* elem, struct list_elem* data)
{
    if (elem == NULL || data == NULL)   return;    
    ASSERT (elem->next != NULL);

    struct list_elem* next = elem->next;
    data->prev = elem;
    data->next = next;
    elem->next = data;
    next->prev = data;
};

void 
list_remove (struct list_elem* elem)
{
    if (elem == NULL)   return;
    ASSERT (elem->next != NULL);
    ASSERT (elem->prev != NULL);

    elem->next->prev = elem->prev;
    elem->prev->next = elem->next;
};

struct list_elem* 
list_front (struct list* list)
{
    if (list == NULL)   return NULL;   

    return list->head.next;
};

struct list_elem* 
list_back (struct list* list)
{
    if (list == NULL)   return NULL;

    return list->tail.prev;
};

struct list_elem* 
list_head (struct list* list)
{
    return list_front (list);
};

struct list_elem* 
list_tail (struct list* list)
{
    return list_back (list);    
};

struct list_elem* 
list_next (struct list_elem* e)
{
    if (e == NULL)  return NULL;

    return e->next;    
};

struct list_elem*
list_prev (struct list_elem* e)
{
    if (e == NULL)  return NULL;

    return e->prev;
};

void
list_insert_ordered (struct list* list, struct list_elem* elem, 
        list_compare_func* func, void* aux)
{
    if (list == NULL || elem == NULL || func == NULL)   return;

    struct list_elem* e;
    for (e = list_front (list); e != list_back (list); e = list_next (e))
    {
        int val = func (e, elem, aux);
        if (val >= 0)
        {
            list_insert_before (e, elem);
            return;
        }
    }
    list_push_back (list, elem);
};
