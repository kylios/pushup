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
