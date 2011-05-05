#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/mongoose.h"
#include "event.h"
#include "main.h"
#include "type.h"
#include "debug.h"
#include "protocol.h"
#include "session.h"
#include "user.h"

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
 * Function that gets called to handle a user action,
 * such as register, unregister, post, or update.
 * */
static user_event_func user_func_table[4];

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
    reqtype_t t = protocol_eval (&pinfo, content, info)
    user_func_table[t] (&pinfo, content, info);

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

/*
 * TODO: implement this one day.
 * Currently no plans to implement this.
 * */
void* mg_init_ssl (struct mg_connection* conn,
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
    uint8 code = 200;

    /*
     * If this user has already registered with a different session,
     * then the user should already be created and in our index.  Look
     * it up first.
     * */
    user = lookup_user (pinfo.u);

    if (user == NULL)
    {
        const char* uid = pinfo.u;
        user = (user_t*) malloc (sizeof (user_t));
        if (user == NULL)
        {
            result = false;
            code = 500;
        }
        else
        {
            /*
             * This adds the user to our global users data structure.
             * */
            user_init (user, uid);
        }
    }
    
    if (result)
    {
        /*
         * Now register this user with the requested session.
         * */
        user_register (user, session);
    }

    r->code = code;
    r->message = "";
    r->success = result;

    return result;
};

bool
user_unregister_func (response_t* r, protocol_info_t* pinfo,
        const char* content, const struct mg_request_info* info)
{
    ASSERT (r);
};

bool
user_push_func (response_t* r, protocol_info_t* pinfo,
        const char* content, const struct mg_request_info* info)
{
    ASSERT (r);
    printf ("result: %d\n", init_events (pinfo->m, NULL));

};

bool 
user_update_func (response_t* r, protocol_info_t* pinfo,
        const char* content, const struct mg_request_info* info)
{
    ASSERT (r);
};
