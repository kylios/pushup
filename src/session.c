#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "user.h"
#include "protocol.h"
#include "type.h"
#include "debug.h"
#include "lib/hash.h"
#include "session.h"

typedef struct
{
    struct list_elem elem;  // Keeps track of this element in the session's 
                            // user-list
    user_t* user;           // The user object 
}   session_user_record_t;


static struct hash session_index;
// TODO: can we build synchronization INTO the hash?  So that multiple
// operations can be performed on it at once, but individual modifications to
// the same bucket or element results in locking
static pthread_mutex_t session_index_lock = PTHREAD_MUTEX_INITIALIZER;


uint32 
session_hash (struct hash_elem* e)
{
    ASSERT (e);    

    session_t* s;
    char* uid;
    uint32 hash = 0;

    s = HASH_ENTRY(e, session_t, elem);
    ASSERT (s);
    // This is a 32 character string
    uid = s->name;
    hash = 0;

    int i;
    for (i = 0; i < SESSION_STR_SZ; i++)
    {
        if (*uid == '\0')
            break;
        hash ^= (*uid) << ((uint32) i % SESSION_STR_SZ);
        uid++;
    }

    return hash;
};

int 
session_compare (struct hash_elem* _a, struct hash_elem* _b, void* aux)
{
    session_t* a = HASH_ENTRY(_a, session_t, elem);
    session_t* b = HASH_ENTRY(_b, session_t, elem);

    ASSERT(a);
    ASSERT(b);

    return strcmp (a->name, b->name);
};

bool 
init_session_index ()
{
    hash_init (&session_index, &session_hash, &session_compare);

    return true;
};


/**
 * Lookup and return a session in the database with the given id.  Returns NULL if
 * no such user exists. 
 * */
session_t* 
lookup_session (const char* id)
{
    printf ("Looking up session %s... ", id);
    /*
     * Temporary read-only session object to compare with.
     * */
    session_t s;
    strncpy (s.name, id, SESSION_STR_SZ);

    /*
     * Synchronized access to index
     * */
    pthread_mutex_lock (&session_index_lock);
    struct hash_elem* e = 
        hash_find (&session_index, &s.elem, NULL);
    pthread_mutex_unlock (&session_index_lock);

    if (e == NULL)
    {
        printf ("Not found\n");
        return NULL;
    }
    printf ("found\n");
    return HASH_ENTRY(e, session_t, elem);
};

/**
 * Initialize a new session structure and add it to the database.  
 * Return FALSE if unsuccessful.
 * */
bool 
session_init (session_t* s, const char* id)
{
    ASSERT(s);

    strncpy (s->name, id, SESSION_STR_SZ);
    pthread_mutex_init (&s->users_lock, NULL);
    list_init (&s->users);

    pthread_mutex_lock (&session_index_lock);
    hash_insert (&session_index, &s->elem);
    pthread_mutex_unlock (&session_index_lock);

    return true;
};

bool
session_add_user (session_t* s, user_t* u)
{
    ASSERT(s);
    ASSERT(u);

    printf ("adding user %s to session %s\n", u->id, s->name);

    /*
     * First check to make sure we don't already have this user in our session.
     * */
    session_user_record_t* record = NULL;
    struct list_elem* e;
    bool found = false;
    pthread_mutex_lock (&s->users_lock);
    printf ("searching for user %s in session list...", u->id);
    for (e = list_begin (&s->users); e != list_end (&s->users); 
            e = list_next (e))
    {
        pthread_mutex_unlock (&s->users_lock);

        record = LIST_ENTRY(e, session_user_record_t, elem);
        ASSERT(record);

        if (record->user == u)
        {
            printf ("found\n");
            found = true;
            break;
        }

        pthread_mutex_lock (&s->users_lock);
    }
    pthread_mutex_unlock (&s->users_lock);

    if (!found)
    {
        printf ("not found\n");
        /*
         * Add user to session's user-list
         * */
        record = (session_user_record_t*) 
            malloc (sizeof (session_user_record_t));
        if (!record)
        {
            return false;
        }
        record->user = u;

        pthread_mutex_lock (&s->users_lock);
        list_push_back (&s->users, &record->elem);
        pthread_mutex_unlock (&s->users_lock);
    }

    return true;
};

void
session_remove_user (session_t* s, user_t* u)
{
    ASSERT(s);
    ASSERT(u);

    printf ("Removing user %s from session %s\n", u->id, s->name);
    session_user_record_t* record = NULL;
    struct list_elem* e;
    bool found = false;
    pthread_mutex_lock (&s->users_lock);
    for (e = list_begin (&s->users); e != list_end (&s->users); 
            e = list_next (e))
    {
        pthread_mutex_unlock (&s->users_lock);

        record = LIST_ENTRY(e, session_user_record_t, elem);
        ASSERT(record);

        if (record->user == u)
        {
            found = true;
            break;
        }

        pthread_mutex_lock (&s->users_lock);
    }
    pthread_mutex_unlock (&s->users_lock);

    if (found)
    {
        pthread_mutex_lock (&s->users_lock);
        list_remove (e);
        pthread_mutex_unlock (&s->users_lock);
    }
};
