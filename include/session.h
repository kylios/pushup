#ifndef SESSION_H
#define SESSION_H

#include "lib/hash.h"

/**
 * session.h defines functions and data structures for managing real-time push
 * sessions.  A session is an interaction between a number of clients.  Clients
 * can broadcast events to the session and request to receive updates from the
 * session.  
 * */

/**
 * Initializes the handler of sessions.
 * */
void init_session_handler ();

typedef struct {
    struct hash_elem elem;
    char name[33];    // Session names must be <= 32 characters long
    unsigned id;      // Session ID.  These should get recycled 
                        // after sessions are destroyed.
   

} session_t;

uint32 session_hash (struct hash_elem* e);
int session_compare (struct hash_elem* a, struct hash_elem* b, void*);

bool init_session_index ();


/**
 * Lookup and return a session in the database with the given id.  Returns NULL if
 * no such user exists. 
 * */
session_t* lookup_session (const char* id);

/**
 * Initialize a new session structure and add it to the database.  
 * Return FALSE if unsuccessful.
 * */
bool session_init (session_t*, const char* id);

#endif //SESSION_H
