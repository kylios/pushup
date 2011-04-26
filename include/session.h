#ifndef SESSION_H
#define SESSION_H

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
      char name[33]   // Session names must be <33 characters long
    , unsigned id    // Session ID.  These should get recycled 
                    // after sessions are destroyed.
   

}

#endif //SESSION_H
