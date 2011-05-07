#include <stdlib.h>
#include <stdio.h>
#include <stdio.h>
#include <pthread.h>

#include "debug.h"
#include "user.h"
#include "lib/hash.h"
#include "protocol.h"
#include "session.h"
#include "event_queue.h"

static struct hash user_index;
// TODO: can we build synchronization INTO the hash?  So that multiple
// operations can be performed on it at once, but individual modifications to
// the same bucket or element results in locking
static pthread_mutex_t user_index_lock = PTHREAD_MUTEX_INITIALIZER;

bool
init_user_index ()
{
    hash_init (&user_index, &user_hash, &user_compare);

    return true;
};

user_t* 
lookup_user (const char* id)
{
    printf ("Looking up user %s... ", id);
    /*
     * Temporary read-only user object to compare with.
     * */
    user_t u;
    strncpy (u.id, id, USER_STR_SZ);

    /*
     * Synchronized access to index
     * */
    pthread_mutex_lock (&user_index_lock);
    struct hash_elem* e = 
        hash_find (&user_index, &u.elem, NULL);
    pthread_mutex_unlock (&user_index_lock);

    if (e == NULL)
    {
        printf ("not found \n");
        return NULL;
    }
    printf ("found\n");
    return HASH_ENTRY(e, user_t, elem);
};

bool 
user_init (user_t* u, const char* id)
{
    ASSERT(u != NULL);

    strncpy (u->id, id, USER_STR_SZ);
    pthread_mutex_init (&u->session_queues_lock, NULL);
    hash_init (&u->session_queues, &event_queue_hash_func, 
            &event_queue_compare_func);

    pthread_mutex_lock (&user_index_lock);
    hash_insert (&user_index, &u->elem);
    pthread_mutex_unlock (&user_index_lock);

    return true;
};

bool 
user_register (user_t* u, session_t* s)
{
    ASSERT(u);
    ASSERT(s);

    event_queue_t* eq = (event_queue_t*) malloc (sizeof (event_queue_t));
    if (eq == NULL)
    {
        return false;
    }

    /*
     * Add user to session.
     * */
    if (session_add_user (s, u))
    {
        /*
         * Construct a queue for events and add it to the user object.
         * */
        event_queue_init (eq, s);

        pthread_mutex_lock (&u->session_queues_lock);
        hash_insert (&u->session_queues, &eq->elem);
        pthread_mutex_unlock (&u->session_queues_lock);

        return true;
    }
    else
    {
        free (eq);
        return false;
    }
};

bool
user_unregister (user_t* u, session_t* s)
{
    ASSERT(u);
    ASSERT(s);

    session_remove_user (s, u);

    /*
     * Find the event_queue associated with this session so we can remove it
     * from the hash.
     * */
    event_queue_t temp;
    // Since it only uses the session member of the struct to get the hash,
    // that's all we have to initialize.
    temp.session = s;

    pthread_mutex_lock (&u->session_queues_lock);
    struct hash_elem* e = hash_find (&u->session_queues, &temp.elem, NULL);
    hash_remove (e);
    pthread_mutex_unlock (&u->session_queues_lock);

    event_queue_t* eq = HASH_ENTRY (e, event_queue_t, elem);
    ASSERT (eq);
    free (eq);

    return true;
};

void 
user_add_event (user_t* u, session_t* s, event_t* e)
{
    ASSERT (u);
    ASSERT (s);
    ASSERT (e);

    event_queue_t temp;
    temp.session = s;

    struct hash_elem* e = hash_find (&u->session_queues, &temp.elem);
    ASSERT (e);
    event_queue_t* eq = HASH_ENTRY (e, event_queue_t, elem);

    event_queue_push (eq, e);
};

event_t* 
user_shift_event (user_t* u, session_t* s)
{
    ASSERT (u);
    ASSERT (s);

    
};

#define HASH_MASK_SIZE 16
const uint8 hash_mask[] = {
    0xae,   0xb1,   0xf8,   0x76,
    0xff,   0x4c,   0x86,   0xef,
    0x61,   0x4b,   0xab,   0xcd,
    0x8c,   0x51,   0xc4,   0x3a
};

uint32 
user_hash (struct hash_elem* e)
{
    user_t* u;
    char* uid;
    uint32 hash = 0;

    u = HASH_ENTRY(e, user_t, elem);
    ASSERT (u);
    // This is a 32 character string
    uid = &u->id[0];
    hash = 0;

    int i;
    for (i = 0; i < USER_STR_SZ; i++)
    {
        if (*uid == '\0')
            break;
        hash ^= (*uid) << ((uint32) i % USER_STR_SZ) | hash_mask[i % HASH_MASK_SIZE];
        uid++;
    }

    return hash;
};

int
user_compare (struct hash_elem* a, struct hash_elem* b, void* AUX)
{
    user_t* _a;
    user_t* _b;

    _a = HASH_ENTRY(a, user_t, elem);
    _b = HASH_ENTRY(b, user_t, elem);
    ASSERT(_a);
    ASSERT(_b);

    return strcmp (_a->id, _b->id);
};

