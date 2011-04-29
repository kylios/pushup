#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <type.h>
#include <pthread.h>

#include "type.h"
#include "event.h"
#include "user.h"
#include "lib/list.h"
#include "debug.h"

/*
 * Keeps track of all the events
 * */
static struct list events;

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

int 
init_events (const char* message, user_t* user)
{
    int num = 0;
    printf ("message: %s\n", message);

    // The message we're given is marked const, so let's copy ourselves a new
    // one since parse_json_events is going to modify the string.
    char* m = (char*) malloc (sizeof (char) * (1 + strlen (message)));
    strcpy (m, message);

    json_parse_state_t state = { 0, NULL };

    const char* str = parse_json_events (m, &state);
    while (NULL != str && num >= 0)
    {
        num++;
        event_t* e = (event_t*) malloc (sizeof (event_t));
        event_init (e, user, str);
        str = parse_json_events (NULL, &state);
    }
    printf ("DONE!\n");

    return num;
};

void 
event_init (event_t* e, user_t* user, const char* message)
{
    ASSERT(e != NULL);

    e->message = message;
    e->reg_count = 0;
    e->sender = user;
    pthread_mutex_init (&e->reg_count_lock, NULL);
};

