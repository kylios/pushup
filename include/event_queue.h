#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include <stddef.h>
#include <stdlib.h>
#include <pthread.h>

#include "type.h"
#include "debug.h"
#include "session.h"
#include "event.h"
#include "lib/list.h"
#include "lib/hash.h"


/**
 * Defines an event queue.  This structure must have a list to act
 * as the queue and a session with which the queue is associated. 
 * We probably want a lock to protect the list.
 * */
typedef struct
{
    struct hash_elem elem;      // For storing in a user_t object

    struct list events;         // Holds all the events 
    pthread_mutex_t events_lock;// Protects events

}   event_queue_t;

void event_queue_init (event_queue_t* eq, session_t*);

#endif //EVENT_QUEUE_H
