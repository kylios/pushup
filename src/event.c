#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <type.h>
#include <pthread.h>

#include "user.h"
#include "type.h"
#include "event.h"
#include "lib/list.h"
#include "debug.h"
#include "protocol.h"

/*
 * Keeps track of all the events
 * */
static struct list events;
static pthread_mutex_t events_lock;

typedef struct
{
    int bdepth;
    char* save_ptr;
} json_parse_state_t;

static const char*
parse_json_events (char* message, json_parse_state_t* state)
{
    // TODO: make this function more elegant!!
    char c;
    char* s;

    if (message == NULL)
    {
        if (state->save_ptr == NULL)
        {
            return NULL;
        }
        message = state->save_ptr;    
    }
    
    s = message;
    while (',' != (c = *message) || state->bdepth != 1)
    {
        if (c == '[')
        {
            state->bdepth++;
            s++;
        }
        else if (c == '{')
        {
            state->bdepth++;
            *message = c;
        }
        else if (c == ']')
        {
            state->bdepth--;
            if (state->bdepth == 0)
            {
                *message = '\0';
                message++;
                return s;
            }
        }
        else if (c == '}')
        {
            state->bdepth--;
            *message = c;
        }
        else if (c == '\0')
        {
            message++;
            return NULL;
        }
        else
        {
            *(message) = c;
        }
        message++;
    }
    *message = '\0';
    state->save_ptr = (message + 1);
    return (const char*) s;
};

void
init_events ()
{
    list_init (&events);
    pthread_mutex_init (&events_lock, NULL);
};

// TODO: this function should be atomic
int 
handle_events (const char* message, user_t* user, session_t* session)
{
    ASSERT (message);
    ASSERT (user);
    ASSERT (session);

    int num = 0;

    // The message we're given is marked const, so let's copy ourselves a new
    // one since parse_json_events is going to modify the string.
    char* m = (char*) malloc (sizeof (char) * MESSAGE_STR_SZ);
    if (!m)
    {
        return -1;
    }
    strncpy (m, message, MESSAGE_STR_SZ);

    json_parse_state_t state = { 0, NULL };

    const char* str = parse_json_events (m, &state);
    while (NULL != str && num >= 0)
    {
        num++;
        event_t* e = event_init (user, str);
        if (!e)
        {
            return -1;
        }

        pthread_mutex_lock (&events_lock);
        list_push_back (&events, &e->allelem);
        pthread_mutex_unlock (&events_lock);

        pthread_mutex_lock (&session->users_lock);
        struct list_elem* elem;
        for (elem = list_front (&session->users); elem != list_back (&session->users); 
                elem = list_next (elem))
        {
            session_user_record_t* record = 
                LIST_ENTRY (elem, session_user_record_t, elem);
            ASSERT (record);
            user_t* u = record->user;           
            ASSERT (u);
            user_add_event (u, session, e);
        }
        pthread_mutex_unlock (&session->users_lock);
        
        str = parse_json_events (NULL, &state);
    }

    return num;
};

event_t*
event_init (user_t* user, const char* message)
{
    event_t* e = (event_t*) malloc (sizeof (event_t));
    if (!e)
    {
        return NULL;
    }

    e->message = message;
    e->reg_count = 0;
    e->sender = user;
    e->message_size = strnlen (message, MESSAGE_STR_SZ);
    pthread_mutex_init (&e->reg_count_lock, NULL);

    return e;
};

void
event_done (event_t* e)
{
    pthread_mutex_lock (&e->reg_count_lock);
    ASSERT (e->reg_count > 0);
    e->reg_count--;

    if (0 == e->reg_count)
    {
        list_remove (&e->allelem);
        free (e);
        return;
    }

    pthread_mutex_unlock (&e->reg_count_lock);
};
