#include "lib/mongoose.h"

/**
 * Defines a handler for a particular mongoose event type.
 * */
typedef void * (*mg_event_func_t) (struct mg_connection*, 
        const struct mg_request_info*);

/**
 * mg_callback_t
 * Callback function for mongoose events.  Handles new requests, errors, logs,
 * and SSL requests.
 * */
void* mg_callback_func (enum mg_event event, struct mg_connection* conn, 
        const struct mg_request_info* info);

/**
 * The handler functions for the mongoose events.
 * */
void* mg_new_request_func (struct mg_connection*,
        const struct mg_request_info*);
void* mg_http_error_func (struct mg_connection*,
        const struct mg_request_info*);
void* mg_event_log_func (struct mg_connection*,
        const struct mg_request_info*);
void* mg_init_ssl (struct mg_connection*,
        const struct mg_request_info*);

