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
        // TODO: in this case, we should create a queue in the hash instead of
        // return an error.
        return false;
    }
    eq = HASH_ENTRY (el, event_queue_t, elem);
    ASSERT (eq);

    // TODO: wait until events come in for us
    int count;
    int num = 0;
    //if (0 == sem_getvalue (&eq->events_count, &count) && count)
    //{
        /*
         * Thread waits until another thread POSTs to the semaphore and
         * increments its count.
         * */
        while (0 == sem_wait (&eq->events_count))
        {
            event = event_queue_shift (eq);

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

            num++;
        }
    //}

    strncat (message, "]", 1);

    return true;
};
