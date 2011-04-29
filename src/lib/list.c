#include <stdio.h>

#include "lib/list.h"
#include "type.h"
#include "debug.h"

void 
list_init (struct list* list)
{
    ASSERT (list != NULL);

    list->head.prev = NULL;
    list->head.next = &list->tail;
    list->tail.prev = &list->head;
    list->tail.next = NULL;
};

void 
list_push_back (struct list* list, struct list_elem* e)
{
    ASSERT (list != NULL);
    ASSERT (e != NULL);

    list_insert_before (list_back (list), e);    
};

void 
list_push_front (struct list* list, struct list_elem* e)
{
    ASSERT(list != NULL);
    ASSERT(e != NULL);

    list_insert_before (list_front (list), e);
};

struct list_elem* 
list_pop_back (struct list* list)
{
    ASSERT (list != NULL);

    struct list_elem* e = list_back (list);
    e = e->prev;
	ASSERT (e->prev != NULL);
    ASSERT (e != &list->head);
    list_remove (e);
    return e;
};

struct list_elem* 
list_pop_front (struct list* list)
{
    ASSERT (list != NULL);

    if (list_empty (list))
        return NULL;

    struct list_elem* e = list_front (list);
    ASSERT (e != NULL);
    ASSERT (e != &list->head);
    list_remove (e);
    return e;
};

void 
list_insert_before (struct list_elem* elem, struct list_elem* data)
{
    ASSERT (data != NULL);
    ASSERT (elem != NULL);
    ASSERT (elem->prev != NULL);
    
    data->prev = elem->prev;
    data->next = elem;
    elem->prev->next = data;
    elem->prev = data;
};

void 
list_insert_after (struct list_elem* elem, struct list_elem* data)
{
    ASSERT (elem != NULL);
    ASSERT (data != NULL);
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
    ASSERT (elem != NULL);
    ASSERT (elem->next != NULL);
    ASSERT (elem->prev != NULL);
    ASSERT (elem->next != elem);
    ASSERT (elem->prev != elem);

    elem->next->prev = elem->prev;
    elem->prev->next = elem->next;
	elem->next = NULL;
	elem->prev = NULL;
};

struct list_elem* 
list_front (struct list* list)
{
    ASSERT (list != NULL);

    return list->head.next;
};

struct list_elem* 
list_back (struct list* list)
{
    ASSERT (list != NULL);

    return &list->tail;
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
list_begin (struct list* list)
{
    return list_front (list);
};

struct list_elem* 
list_end (struct list* list)
{
    return list_back (list);
};

struct list_elem* 
list_next (struct list_elem* e)
{
    ASSERT (e != NULL);
    ASSERT (e->next != NULL);
    ASSERT (e->prev != NULL);

    return e->next;    
};

struct list_elem*
list_prev (struct list_elem* e)
{
    ASSERT (e != NULL);
    ASSERT (e->next != NULL);
    ASSERT (e->prev != NULL);

    return e->prev;
};

bool 
list_empty (struct list* list)
{
    ASSERT (list != NULL);

    return list_front (list) == list_back (list);
};

struct list_elem* 
list_find (struct list* list, struct list_elem* elem, 
        list_compare_func* func, void* aux)
{
    struct list_elem* e;
    uint32 val;

    ASSERT (list != NULL);    
    ASSERT (elem != NULL);
    ASSERT (func != NULL);
    
    for (e = list_head (list); e != list_tail (list); e = list_next (e))
    {
        val = func (e, elem, aux);

        if (val == 0)
            return e;
    }

    return NULL;
};


void
list_insert_ordered (struct list* list, struct list_elem* elem, 
        list_compare_func* func, void* aux)
{
    ASSERT (list != NULL);
    ASSERT (elem != NULL);
    ASSERT (func != NULL);

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

void 
list_print (struct list* list, list_print_func* func)
{
	struct list_elem* e;

	ASSERT (list != NULL);
	ASSERT (func != NULL);

	for (e = list_head (list); e != list_tail (list); e = list_next (e))
	{
//        framebuf_printf ("--------%p\n", e);
//		framebuf_printf ("Next: %p\n", e->next);
//		framebuf_printf ("Prev: %p\n", e->prev);
		func (e);
//		framebuf_printf ("\n");
	}
};
