#ifndef EVENT_H
#define EVENT_H

#include <pthread.h>

#include "user.h"
#include "session.h"
#include "lib/list.h"

typedef struct 
{
    struct list_elem allelem;
    struct list_elem elem;  // For the event_queue

    pthread_mutex_t reg_count_lock;
    int reg_count;  // Number of registered subscribers.  When this becomes
                    // zero, it should be safe to free this object from memory

    const char* message;
    size_t message_size;    // Number of characters in the message, not 
                            // including NULL terminator

    user_t* sender; // Pointer to the user who broadcasted the event

} event_t;

void init_events ();

/**
 * Initialize a list of events from a json encoded message and add them all to
 * the global list.  Return a count of events initialized, or -1 if an error
 * occurred. 
 * */
int handle_events (const char* message, user_t*, session_t*);




/**
 * Create and return a new event_t object
 * */
event_t* event_init (user_t*, const char*);

/**
 * Checks if this event is in any other queues.  If not, free the space
 * allocated for this event and remove it from any lists.
 * */
void event_done (event_t*);

#endif // EVENT_H
