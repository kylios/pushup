#include <stddef.h>
#include <stdlib.h>
#include <pthread.h>

#include "type.h"
#include "debug.h"
#include "event_queue.h"
#include "lib/list.h"
#include "session.h"

void
event_queue_init (event_queue_t* eq, session_t* s)
{
    ASSERT(eq);

    list_init (&eq->events);
    pthread_mutex_init (&eq->events_lock, NULL);

    eq->session = s;
};

uint32 
event_queue_hash_func (struct hash_elem* e)
{
    ASSERT (e);
    event_queue_t* eq = HASH_ENTRY (e, event_queue_t, elem);
    ASSERT (eq);
    ASSERT (eq->session);

    return session_hash (&eq->session->elem);
};

int 
event_queue_compare_func (struct hash_elem* _a, struct hash_elem* _b, 
        void* AUX)
{
    return 0;
};

void
event_queue_push (event_queue_t* eq, event_t* e)
{
    ASSERT (eq);
    ASSERT (e);

    pthread_mutex_lock (&eq->events_lock);
    list_push_back (&eq->events, &e->elem);
    pthread_mutex_unlock (&eq->events_lock);
};

event_t*
event_queue_shift (event_queue_t* eq)
{
    ASSERT (eq);

    pthread_mutex_lock (&eq->events_lock);
    struct list_elem* e = list_front (&eq->events);
    ASSERT (e);
    list_remove (e);
    pthread_mutex_unlock (&eq->events_lock);

    event_t* event = LIST_ENTRY (e, event_t, elem);
    ASSERT (event);

    return event;
};


