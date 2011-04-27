#ifndef EVENT_H
#define EVENT_H

#include "user.h"

typedef struct {
    int reg_count;   // Number of registered subscribers.  When this becomes
                        // zero, it should be safe to free this object from memory

    char* message;   // The message, as a text blob.  If you need specific 
    size_t length;   // parameters in your message, it's reccommended to 
                        // format the message as a json object.

    user_t* sender;  // Pointer to the user who broadcasted the event

} event_t;

/**
 * Initialize a list of events from a json encoded message and add them all to
 * the global list.  Return a count of events initialized, or -1 if an error
 * occurred. 
 * */
int init_events (const char* message);




void event_init (event_t*, char*, size_t);

#endif // EVENT_H
