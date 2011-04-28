#include <stdlib.h>
#include <stdio.h>
#include <type.h>

#include "type.h"
#include "event.h"
#include "lib/list.h"

/*
 * Keeps track of all the events
 * */
static struct list events;

typedef struct
{
    int bdepth;
    char* save_ptr;
} json_parse_state_t;

static char*
parse_json_events (char* message, json_parse_state_t* state)
{
    char c;
    char* s;

    if (message == NULL)
    {
        message = state->save_ptr;    
    }
    
    s = message;
    while (',' != (c = *message) || state->bdepth != 1)
    {
        if (isspace (c))
        {
            continue;
        }
        else if (c == '[')
        {
            state->bdepth++;
        }
        else if (c == ']')
        {
            state->bdepth--;
        }
        else if (*message == '\0')
        {
            return NULL;
        }
        else
        {
            *(state++) = c;
        }
    }
    *message = '\0';
    state->save_ptr = (message + 1);
    return s;
};

int 
init_events (const char* message)
{
    printf ("message: %s\n", message);


};
