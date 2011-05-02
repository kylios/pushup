#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "debug.h"
#include "user.h"
#include "lib/hash.h"
#include "protocol.h"

static struct hash user_index;
// TODO: can we build synchronization INTO the hash?  So that multiple
// operations can be performed on it at once, but individual modifications to
// the same bucket or element results in locking
static pthread_mutex_t user_index_lock = PTHREAD_MUTEX_INITIALIZER;

bool
init_user_index ()
{
    hash_init (&user_index, &user_hash, &user_compare);
};

user_t* 
lookup_user (const char* id)
{
    
};

bool 
user_init (user_t* u, const char* id)
{
    ASSERT(u != NULL);

//    u->elem = HASH_ELEM_INITIALIZER;
    strcpy (&(u->id[0]), id);

    pthread_mutex_lock (&user_index_lock);
    hash_insert (&user_index, &u->elem);
    pthread_mutex_unlock (&user_index_lock);
};

bool 
user_register (user_t* u, const char* sess)
{
    
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
        hash ^= (*uid) << ((uint32) i % USER_STR_SZ);
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

    return strcmp (&(_a->id[0]), &(_b->id[1]));
};

