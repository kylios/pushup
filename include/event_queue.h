#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include <stddef.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#include "user.h"
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
    pthread_cond_t events_cond; // Signal an updating thread to wake up
    int events_count;           // Counts the events in the queue

    session_t* session;

}   event_queue_t;

uint32 event_queue_hash_func (struct hash_elem*);
int event_queue_compare_func (struct hash_elem*, struct hash_elem*, void*);

void event_queue_init (event_queue_t* eq, session_t* s);

bool event_queue_push (event_queue_t* eq, event_t*);
void event_queue_signal (event_queue_t* eq);
event_t* event_queue_shift (event_queue_t*, int* num_left);

void event_queue_debug (event_queue_t* eq);

#endif //EVENT_QUEUE_H
