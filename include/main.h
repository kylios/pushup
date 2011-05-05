#include "lib/mongoose.h"
#include "type.h"
#include "protocol.h"



typedef struct
{
    bool success;
    uint8 code;
    const char* message;
} response_t;

/**
 * Defines a handler for a particular mongoose event type.
 * */
typedef void * (*mg_event_func_t) (struct mg_connection*, 
        const struct mg_request_info*);

/**
 * Defines a handler for different user request types.
 * */
typedef bool (*user_event_func) 
        (response_t*, protocol_info_t*, const char*, 
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

bool user_register_func (response_t*, protocol_info_t*, 
        const char*, const struct mg_request_info*);
bool user_unregister_func (response_t*, protocol_info_t*, 
        const char*, const struct mg_request_info*);
bool user_push_func (response_t*, protocol_info_t*, 
        const char*, const struct mg_request_info*);
bool user_update_func (response_t*, protocol_info_t*, 
        const char*, const struct mg_request_info*);
