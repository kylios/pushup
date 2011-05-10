#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>

/*
 * Callback for when threads become suspended due to asynchronous connections.  
 * When a thread relinquishes control of a connection, it'll call this function.
 * */
typedef void* mg_suspended_callback_t (struct mg_connection *conn,
                                const struct mg_request_info *request_info);

typedef struct
{
    pthread_mutex_t* lock;
} thread_pool_cond_t;

typedef struct
{
    
} thread_pool_task_t;

#endif //THREAD_POOL_H
