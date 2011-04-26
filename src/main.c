#include <stdio.h>
#include <stdlib.h>

#include "lib/mongoose.h"
#include "main.h"
#include "type.h"
#include "debug.h"
#include "protocol.h"

/**
 * Context in which all mongoose functions operate.
 * */
static struct mg_context* context;

/**
 * Mongoose options for the server.
 * */
static const char* options[] = {
        "access_log_file", "logs/access"
    ,   "listening_ports", "8082"
    ,   "num_threads", "5"
    ,   NULL
};
//static const char** options = NULL;

/**
 * Table of event handler functions to be invoked when the corresponding event
 * is passed to mg_callback_func.
 * */
static mg_event_func_t event_func_table[4];

static const char* response_header = 
    "HTTP/1.1 200 OK\r\n"
    "Cache: no-cache\r\n"
    "Content-Type: text/html\r\n"
    "\r\n";

int
main (int argc, char** argv)
{
    event_func_table[MG_NEW_REQUEST] = &mg_new_request_func;
    event_func_table[MG_HTTP_ERROR] = &mg_http_error_func;
    event_func_table[MG_EVENT_LOG] = &mg_event_log_func;
    event_func_table[MG_INIT_SSL] = &mg_init_ssl;

    context = mg_start (&mg_callback_func, options);
    if (context == NULL)
    {
        printf ("fail\n");
        exit (1);
    }

    while (1);
}

void* 
mg_callback_func (enum mg_event event, struct mg_connection* conn, 
        const struct mg_request_info* info)
{
    (event_func_table[event])(conn, info);
    return (void*)1;
};

void* mg_new_request_func (struct mg_connection* conn,
        const struct mg_request_info* info)
{
    /*
     * Figure out what they want to do with this request
     * */
    protocol_info_t pinfo;
    switch (protocol_eval (&pinfo, info))
    {
        case PR_PUSH:
            printf ("push\n");
            break;
        case PR_UPDATE:
            printf ("update\n");
            break;
        case PR_REG:
            printf ("register\n");
            break;
        case PR_UREG:
            printf ("unregister\n");
            break;
        default:
            printf ("nothing\n");
            break;
    }
    




    printf ("received new request\n");
    mg_printf (conn, "%shello\n", response_header);
    return NULL;
};

void* mg_http_error_func (struct mg_connection* conn,
        const struct mg_request_info* info)
{
    
    return NULL;
};

void* mg_event_log_func (struct mg_connection* conn,
        const struct mg_request_info* info)
{
    
    return NULL;
};

void* mg_init_ssl (struct mg_connection* conn,
        const struct mg_request_info* info)
{
    
    return NULL;
};

