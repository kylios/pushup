#include <stddef.h>

#include "debug.h"
#include "user.h"
#include "session.h"
#include "event.h"
#include "event_queue.h"

bool
register_user_session (const char* uid, const char* sid,
    user_t** u, session_t** s)
{
    ASSERT (uid);
    ASSERT (sid);

    user_t* user;
    session_t* session;

    user = get_or_init_user (uid);   
    if (user)
    {
        session = get_or_init_session (sid);
        
        if (session)
        {
            /*
             * Now register this user with the requested session.
             * */
            user_register (user, session);
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    if (u)  *u = user;
    if (s)  *s = session;

    return true;
};

void
unregister_user_session (const char* uid, const char* sid)
{
    ASSERT (uid);
    ASSERT (sid);

    user_t* user = lookup_user (uid);
    session_t* session = lookup_session (sid);

    if (user == NULL)
    {
        return;
    }
    if (session == NULL)
    {
        return;
    }

    user_unregister (user, session);
};

bool
push_user_session (const char* uid, const char* sid, const char* message, 
        user_t** u, session_t** s)
{
    ASSERT (uid);
    ASSERT (sid);
    ASSERT (message);

    user_t* user = lookup_user (uid);
    session_t* session = lookup_session (sid);

    if (user && u)  *u = user;
    if (session && s)   *s = session;

    if (user && session && -1 < handle_events (message, user, session))
    {
        return true;
    }
    else
    {
        return false;
    }
};

bool 
update_user_session (const char* uid, const char* sid, char* message, 
        user_t** u, session_t** s)
{
    ASSERT (uid);
    ASSERT (sid);
    ASSERT (message);

    *message = '[';
    *(message + 1) = '\0';

    user_t* user;
    session_t* session;
    event_queue_t* eq;
    event_t* event;

    user = lookup_user (uid);
    session = lookup_session (sid);

    if (u)  *u = user;
    if (s)  *s = session;

    if (!user || !session)
    {
        return false;
    }
    
    event_queue_t temp;
    temp.session = session;
    struct hash_elem* el = hash_find (&user->session_queues, &temp.elem, NULL);
    if (el == NULL)
    {
        user_register (user, session);
        el = hash_find (&user->session_queues, &temp.elem, NULL);
        ASSERT (el);
    }
    eq = HASH_ENTRY (el, event_queue_t, elem);
    ASSERT (eq);

    // TODO: maybe just skip this check?
    pthread_mutex_lock (&eq->events_lock);
    while (list_empty (&eq->events))
    {
        pthread_mutex_unlock (&eq->events_lock);

        // Stop this thread now
        thread_pool_relinquish_thread ();

        return NULL;
    }
    pthread_mutex_unlock (&eq->events_lock);

    /*
     * Call the callback function that the thread *would* have called had this
     * thread been blocked.*/
    user_update_func_cb (user, session, eq);

    return true;
};

void
user_update_func_cb (user_t* user, session_t* session, event_queue_t* eq)
{
    ASSERT (user);
    ASSERT (session);
    ASSERT (eq);
    
    pthread_mutex_lock (&eq->events_lock);
    while (list_empty (&eq->events))
    {
        pthread_mutex_unlock (&eq->events_lock);


        pthread_mutex_lock (&eq->events_lock);
    }
    pthread_mutex_unlock (&eq->events_lock);


    event = event_queue_shift (eq, &num_left);

    /*
     * Concatenate this text to events_content
     * */
    if (num > 0)  strncat (message, ",", 1);
    strncat (message, event->message, event->message_size);

    event_done (event);
    
    if (sem_getvalue (&eq->events_count, &count) || 0 >= count)
    {
        break;
    }


    strncat (message, "]", 1);

};
