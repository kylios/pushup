#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "user.h"
#include "lib/mongoose.h"
#include "event.h"
#include "main.h"
#include "type.h"
#include "debug.h"
#include "protocol.h"
#include "session.h"
#include "event_queue.h"
#include "functions.h"

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
    ,   "num_threads", "5"              // For listening
    ,   "num_processing_threads", "10"  // For processing requests
    ,   NULL
};
//static const char** options = NULL;

/**
 * Function that gets called to handle a user action,
 * such as register, unregister, post, or update.
 * */
static user_event_func user_func_table[5];

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

    user_func_table[PR_REG] = &user_register_func;
    user_func_table[PR_UREG] = &user_unregister_func;
    user_func_table[PR_PUSH] = &user_push_func;
    user_func_table[PR_UPDATE] = &user_update_func;
    user_func_table[PR_DEBUG] = &user_debug_func;

    init_user_index ();
    init_session_index ();
    init_events ();

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
        const struct mg_request_info* info, int* result)
{
    *result = 0;
    if ((event_func_table[event])(conn, info))
    {
        *result = 1;
    }
    return (void*)1;
};

void* 
mg_new_request_func (struct mg_connection* conn,
        const struct mg_request_info* info)
{
    /*
     * Get the request body
     * */
    char* _content = NULL;
    size_t content_length = 1;

    // If the request is not POST, then we don't need to bother
    if (strcmp (info->request_method, "POST") == 0)
    {
        const char* str_content_length = mg_get_header (conn, "Content-Length");
        if (str_content_length != NULL)
        {
            content_length += atoi (str_content_length);

            printf ("Content-Length: %u\n", content_length);
            _content = (char*) malloc (sizeof (char) * content_length);
            if (_content != NULL)
            {
                mg_read (conn, (void*) _content, sizeof (char) * content_length);
            }
        }
    }
    _content[content_length - 1] = '\0';
    const char* content = (const char*) _content;

    /*
     * Figure out what they want to do with this request
     * */
    protocol_info_t pinfo;
    response_t response;
    reqtype_t t = protocol_eval (&pinfo, content, info);
    bool result = user_func_table[t] (&response, &pinfo, content, info);

    printf ("received new request\n");
    mg_printf (conn, "%s%s\n", response_header, response.message);
    return (!result ? 0xffffffff : NULL);
};

void* 
mg_http_error_func (struct mg_connection* conn,
        const struct mg_request_info* info)
{
    
    return NULL;
};

void* 
mg_event_log_func (struct mg_connection* conn,
        const struct mg_request_info* info)
{
    
    return NULL;
};

/*
 * TODO: implement this one day.
 * Currently no plans to implement this.
 * */
void* 
mg_init_ssl (struct mg_connection* conn,
        const struct mg_request_info* info)
{
    return NULL;
};

bool 
user_register_func (response_t* r, protocol_info_t* pinfo, 
        const char* content, const struct mg_request_info* info)
{
    ASSERT (r);

    user_t* user;
    session_t* session;
    bool result = true;
    uint16 code = 200;
    const char* message = "OK";

    if (!register_user_session (pinfo->u, pinfo->s, &user, &session))
    {
        code = 500;
        message = "Server ran out of memory";
        result = false;
    }

    r->code = code;
    r->message = message;
    r->success = result;

    return true;
};

bool
user_unregister_func (response_t* r, protocol_info_t* pinfo,
        const char* content, const struct mg_request_info* info)
{
    ASSERT (r);

    r->code = 200;
    r->success = true;
    r->message = "OK";

    unregister_user_session (pinfo->u, pinfo->s);

    return true;
};

bool
user_push_func (response_t* r, protocol_info_t* pinfo,
        const char* content, const struct mg_request_info* info)
{
    ASSERT (r);
    ASSERT (pinfo);
    ASSERT (content);
    ASSERT (info);

    user_t* user;
    session_t* session;

    printf ("message: %s\n", pinfo->m);

    if (!push_user_session (pinfo->u, pinfo->s, pinfo->m, &user, &session))
    {
        if (!user)
        {
            r->code = 200;
            r->message = "ERROR: user does not exist";
            r->success = false;
            return true;
        }
        else if (!session)
        {
            r->code = 200;
            r->message = "ERROR: session does not exist";
            r->success = false;
            return true;
        }
        else
        {
            r->code = 200;
            r->message = "Failed to import events";
            r->success = false;
            return true;
        }
    }

    r->code = 200;
    r->message = "OK";
    r->success = true;

    return true;
};

bool 
user_update_func (response_t* r, protocol_info_t* pinfo,
        const char* content, const struct mg_request_info* info)
{
    ASSERT (r);
    ASSERT (pinfo);
    ASSERT (content);
    ASSERT (info);

    // The json text of the events that we'll return to the client
    char* events_content;
    event_t* event;
    event_queue_t* eq;
    user_t* user = lookup_user (pinfo->u);
    session_t* session = lookup_session (pinfo->s);

    events_content = (char*) malloc (sizeof (char) * MESSAGE_STR_SZ);
    if (!events_content)
    {
        r->code = 500;
        r->message = "Server ran out of memory";
        r->success = false;
        return true;
    }

    if (!update_user_session (pinfo->u, pinfo->s, events_content, &user, &session))
    {
        if (!user)
        {
            r->code = 200;
            r->message = "ERROR: user does not exist";
            r->success = false;
            return true;
        }
        if (!session)
        {
            r->code = 200;
            r->message = "ERROR: session does not exist";
            r->success = false;
            return true;
        }

        /*
         * If this is the case, we have to relinquish control of this thread.  
         * update_user_session should have stored the thread state in the global
         * data structure, so all we need to do is tell mongoose to give up that
         * task for the time being.
         * */
        return false;
    }

    r->code = 200;
    r->message = events_content;
    r->success = true;

    return true;
};

bool
user_debug_func (response_t* r, protocol_info_t* pinfo,
        const char* content, const struct mg_request_info* info)
{
    printf ("debug\n");
    /*
     * Just output all the users, sessions, and events.
     * */
    struct hash_iter iter;
    user_debug ();

    return true;
};

