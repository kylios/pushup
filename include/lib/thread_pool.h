#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>

#include "type.h"
#include "lib/mongoose.h"
#include "user.h"
#include "session.h"
#include "event_queue.h"

/*
 * Callback for when threads become suspended due to asynchronous connections.  
 * When a thread relinquishes control of a connection, it'll call this function.
 * */
typedef void* mg_suspended_callback_t (struct mg_connection *conn,
                                const struct mg_request_info *request_info);

/*
 * The thread_pool_t structure needs to define a list to hold pthreads and a
 * list to hold tasks, and also a semaphore counting the number of tasks that
 * are free to assume.  Threads will all wait for the semaphore to POST, which
 * the listening threads will do when new tasks come in.  
 *
 * When a thread obtains a task, it can be in two states: new task, and in
 * progress task.  New tasks will be basically brand new requests from the
 * listening threads that need to be picked up and executed.  In progress tasks
 * are tasks that have been waiting on an update, finally received data, and
 * need to be carried out.  The state should be indicated by the thread_state_t
 * enumerated type.
 *
 * */

enum thread_state_t
{
    THREAD_STATE_NEW,
    THREAD_STATE_WAITING
};

typedef struct
{
    struct list_elem elem;

    enum thread_state_t state;
    enum mg_event event;
    struct mg_connection* conn;
    const struct mg_request_info* info;

    /* State variables used only when this thread is resumed */
    user_t* user;
    session_t* session;
    event_queue_t* eq;
} thread_pool_task_t;

bool init_thread_pool (int num_processing_threads);
bool thread_pool_add_new_task (enum mg_event, struct mg_connection*, const struct mg_request_info*);
bool thread_pool_add_in_progress_task (struct mg_connection*, const struct mg_request_info*, 
        user_t*, session_t*, event_queue_t*);
thread_pool_task_t* thread_pool_get_task ();
void thread_pool_free_task (thread_pool_task_t*);

#endif //THREAD_POOL_H
