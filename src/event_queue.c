#include <stddef.h>
#include <stdlib.h>
#include <pthread.h>

#include "type.h"
#include "debug.h"
#include "event_queue.h"
#include "lib/list.h"


void
event_queue_init (event_queue_t* eq)
{
    ASSERT(eq);
    ASSERT(s);

    list_init (&eq->events);
    lock_init (&eq->events_lock);
    eq->sesson = session;
};

