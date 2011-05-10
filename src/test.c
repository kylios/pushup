#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#include "functions.h"
#include "user.h"
#include "type.h"
#include "debug.h"
#include "session.h"
#include "protocol.h"

static void
test1 ()
{
    // TEST 1
    register_user_session ("user1", "session1", NULL, NULL);
    user_debug ();

    // TEST 4
    push_user_session ("user1", "session1", "[message1]", NULL, NULL);
    user_debug ();

    // TEST 2
    register_user_session ("user2", "session1", NULL, NULL);
    register_user_session ("user1", "session2", NULL, NULL);
    user_debug ();

    // TEST 3
    register_user_session ("user2", "session2", NULL, NULL);
    user_debug ();

    // TEST 5
    push_user_session ("user1", "session1", "[message2,message3, message4 ]", NULL, NULL);
    user_debug ();

    push_user_session ("user1", "session2", "[ message5 ]", NULL, NULL);
    push_user_session ("user2", "session1", "[ message6, message7 ]", NULL, NULL);
    push_user_session ("user2", "session2", "[ message8, message9, message10 ]", NULL, NULL);

    user_debug ();

    printf ("\n\nNow begin fetching events\n");
    char* content = (char*) malloc (sizeof (char) * MESSAGE_STR_SZ);

    update_user_session ("user1", "session1", content, NULL, NULL);
    printf ("Message: %s\n", content);
    user_debug ();

    update_user_session ("user2", "session1", content, NULL, NULL);
    printf ("Message: %s\n", content);
    user_debug ();

    update_user_session ("user1", "session1", content, NULL, NULL);
    printf ("Message: %s\n", content);
    user_debug ();

};

struct test2_data
{
    sem_t sem;
    int num_clients;
    int num_sessions;
    int num_sessions_per_client;
    int num_messages;

    pthread_mutex_t lock;
    int client_number;
    int sessions_left;
};

static void*
test2_func (void* data)
{
    if (!data)
        return NULL;
    struct test2_data* d = (struct test2_data*) data;

    // Get set up
    pthread_mutex_lock (&d->lock);
    int uid_num = d->client_number++;
    pthread_mutex_unlock (&d->lock);

    char* uid = (char*) malloc (sizeof (char) * 32);
    sprintf (uid, "user%d\0", uid_num);

    int sessions_created = 0;
    pthread_mutex_lock (&d->lock);
    while (d->sessions_left && sessions_created < d->num_sessions_per_client)
    {
        int sid_num = d->num_sessions - d->sessions_left--;
        pthread_mutex_unlock (&d->lock);

        char* sid = (char*) malloc (sizeof (char) * 32);
        sprintf (sid, "session%d\0", sid_num);

        register_user_session (uid, sid, NULL, NULL);

        pthread_mutex_lock (&d->lock);
    }
    pthread_mutex_unlock (&d->lock);

    sem_wait (&d->sem);

    // Do the stuff
//    pthread_exit (NULL);
};

static void
test2 ()
{
    /*
     * Let's try to simulate a number of clients trying to push messages and
     * update themselves all at the same time.  We can configure the test to
     * increase the number of clients and the number of messages they transmit,
     * as well as the frequency they push and update.  */

    // Number of clients to simulate
    int num_clients = 5;
    // Number of sessions to create.  Clients will not join all the sessions
    int num_sessions = 10;
    // Number of sessions to be joined by each client.
    int num_sessions_per_client = 2;
    // messages pushed by each client
    int num_messages = 30;

    struct test2_data data;
    sem_init (&data.sem, 0, 0);
    data.num_clients = num_clients;
    data.num_sessions = num_sessions;
    data.num_sessions_per_client = num_sessions_per_client;
    data.num_messages = num_messages;
    data.client_number = 0;
    data.sessions_left = num_sessions;
    pthread_mutex_init (&data.lock, NULL);

    pthread_t* threads = (pthread_t*) malloc (sizeof (pthread_t) * num_clients);
    pthread_t* t = threads;
    if (!threads)
        return;
    int i;
    for (i = 0; i < num_clients; i++, threads++)
    {
        pthread_create (threads, NULL, &test2_func, &data);
    }

    for (i = 0; i < num_clients; i++)
    {
        sem_post (&data.sem);
    }

//    threads = t;
//    for (i = 0; i < num_clients; i++, threads++)
//    {
//        void* val;
//        pthread_join (threads, &val);
//    }
};

int
main (int argc, char** argv)
{
    init_user_index ();
    init_session_index ();
    init_events ();

    //test1 ();
    test2 ();
};
