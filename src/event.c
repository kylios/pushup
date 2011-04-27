#include <stdlib.h>
#include <stdio.h>

#include <yajl/yajl_tree.h>

#include "type.h"
#include "event.h"
#include "lib/list.h"

/*
 * Keeps track of all the events
 * */
static struct list events;

static bool
yajl_val_to_event (event_t* e, yajl_val obj)
{

};


int 
init_events (const char* message)
{
    printf ("message: %s\n", message);
    char errbuf[1024];
    yajl_val node = yajl_tree_parse (message, errbuf, sizeof (errbuf));

    if (node == NULL || !YAJL_IS_ARRAY(node))
    {
        return -1;
    }

    /*
     * Iterate through the json array, saving each object as a new event
     * */
    yajl_val* vals = YAJL_GET_ARRAY(node)->values;
    size_t len = YAJL_GET_ARRAY(node)->len;
    int i;
    for (i = 0; i < len; i++)
    {
        yajl_val v = vals[i];
        if (!YAJL_IS_OBJECT(v))
        {
            return -1;
        }
        printf ("This event is good\n");
    }
};
