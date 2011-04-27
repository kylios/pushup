#ifndef EVENT_H
#define EVENT_H

#include <pthread.h>

#include "user.h"

typedef struct 
{
    pthread_mutex_t reg_count_lock;
    int reg_count;  // Number of registered subscribers.  When this becomes
                    // zero, it should be safe to free this object from memory

    const char* message;
    size_t len;

    user_t* sender; // Pointer to the user who broadcasted the event

} event_t;

/**
 * Initialize a list of events from a json encoded message and add them all to
 * the global list.  Return a count of events initialized, or -1 if an error
 * occurred. 
 * */
int init_events (const char* message);




void event_init (event_t*, char*, size_t);

#endif // EVENT_H
