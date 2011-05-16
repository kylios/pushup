#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>

#include "type.h"
#include "debug.h"
#include "lib/thread_pool.h"
#include "lib/mongoose.h"
#include "lib/list.h"

pthread* threads;

static void* thread_func (void*);

static sem_t jobs_count;
static pthread_mutex_t jobs_lock;
static struct list jobs;

bool
init_thread_pool (int num_processing_threads)
{
    if (-1 == sem_init (&jobs_count, 0, 0))
    {
        return false;
    }

    pthread_mutex_init (&jobs_lock, NULL);
    list_init (&jobs);

    threads = (pthread*) malloc (sizeof (pthread) * num_processing_threads);
    if (NULL == threads)
    {
        return false;
    }
    t = threads;

    int i;
    for (i = 0; i < num_processing_threads; i++)
    {
        pthread_create (t, NULL, thread_func, NULL);
        t++;
    }
};

static void*
thread_func (void* AUX)
{
    while (1)
    {
        /*
         * Wait for a job to become available.
         * */
        sem_wait (&jobs_count);

        pthread_mutex_lock (&jobs_lock);
        /*
         * A double-check is necessary in case another thread stole our job from
         * us after we woke up.
         * */
        if (!list_empty (&jobs))
        {
            struct list_elem* e = list_front (&jobs);
            list_remove (e);
            pthread_mutex_unlock (&jobs_lock);


        }
        else
        {
            pthread_mutex_unlock (&jobs_lock);
        }
    }
};

void 
thread_pool_relinquish_thread (struct mg_connection* conn, 
        const struct mg_request_info* info,
        user_t* user, session_t* session, event_queue_t* eq)
{
    ASSERT (conn);
    ASSERT (info);
    ASSERT (user);
    ASSERT (session);
    ASSERT (eq);

    thread_pool_task_t* t = thread_pool_add_in_progress_task (conn, info,
            user, session, eq);
};

static inline thread_pool_task_t*
create_new_task ()
{
    thread_pool_task_t* t = (thread_pool_task_t*) malloc (sizeof (thread_pool_task_t));
    if (t == NULL)
    {
        return NULL;
    }

    return t;
};

bool 
thread_pool_add_new_task (enum mg_event event, struct mg_connection* conn, 
        const struct mg_request_info* info)
{
    ASSERT (conn);
    ASSERT (info);

    thread_pool_task_t* t = create_new_task ();
    if (NULL == t)
    {
        return false;
    }
    t->state = THREAD_STATE_NEW;
    t->event = event;
    t->conn = conn;
    t->info = info;
    t->user = NULL;
    t->session = NULL;
    t->eq = NULL;

    pthread_mutex_lock (&jobs_lock);
    list_push_back (&jobs, &t->elem);
    pthread_mutex_unlock (&jobs_lock);
    sem_post (&jobs_count);

    return true;
};

bool 
thread_pool_add_in_progress_task (struct mg_connection* conn, 
        const struct mg_request_info* info, 
        user_t* user, session_t* session, event_queue_t* eq)
{
    ASSERT (conn);
    ASSERT (info);
    ASSERT (user);
    ASSERT (session);
    ASSERT (eq);

    thread_pool_task_t* t = create_new_task ();
    if (NULL == t)
    {
        return false;
    }
    t->state = THREAD_STATE_WAITING;
    t->conn = conn;
    t->info = info;
    t->user = user;
    t->session = session;
    t->eq = eq;

    pthread_mutex_lock (&jobs_lock);
    list_push_back (&jobs, &t->elem);
    pthread_mutex_unlock (&jobs_lock);
    sem_post (&jobs_count);

    return true;
};

thread_pool_task_t* 
thread_pool_get_task ()
{
    pthread_mutex_lock (&jobs_lock);
    if (!list_empty (&jobs))
    {
        struct list_elem* e = list_front (&jobs);
        list_remove (e);
        pthread_mutex_unlock (&jobs_lock);
        return LIST_ENTRY (e, thread_pool_task_t, elem);
    }
    else
    {
        pthread_mutex_unlock (&jobs_lock);
        return NULL;
    }
};


