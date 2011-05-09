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
    sem_init (&eq->events_count, 0, 0);

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

struct eq_event
{
    struct list_elem elem;
    event_t* e;
};

bool
event_queue_push (event_queue_t* eq, event_t* e)
{
    ASSERT (eq);
    ASSERT (e);

    struct eq_event* ev = 
        (struct eq_event*) malloc (sizeof (struct eq_event));
    if (!ev)
    {
        return false;
    }

    ev->e = e;
    pthread_mutex_lock (&eq->events_lock);
    list_push_back (&eq->events, &ev->elem);
    pthread_mutex_unlock (&eq->events_lock);
    sem_post (&eq->events_count);

    printf ("pushed: %s\n", e->message);

    return true;
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

    struct eq_event* ev = LIST_ENTRY(e, struct eq_event, elem);
    ASSERT (ev);

    event_t* event = ev->e;
    ASSERT (event);
    free (ev);

    return event;
};

static void
event_queue_debug_helper (struct list_elem* e, void* AUX)
{
    ASSERT (e);

    struct eq_event* ev = LIST_ENTRY (e, struct eq_event, elem);
    event_t* event = ev->e;
    ASSERT (event != NULL);

    printf ("  event: '%s', originator: %s\n", event->message, event->sender->id);
};

void 
event_queue_debug (event_queue_t* eq)
{
    ASSERT (eq);    

    printf ("-Session: %s\n", eq->session->name);

    pthread_mutex_lock (&eq->events_lock);
    list_apply (&eq->events, &event_queue_debug_helper);    
    pthread_mutex_unlock (&eq->events_lock);
};
