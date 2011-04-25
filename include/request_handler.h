#ifndef __REQUEST_HANDLER_H
#define __REQUEST_HANDLER_H

#include "lib/mongoose.h"
#include "types.h"

typedef struct request  {
    mg_connection* conn

} request_t;

/**
 * Initialize a new reqeust.  Supply a request_t object, which should be
 * uninitialized.
 * */
bool request_init (request_t*, struct mg_connection*, struct mg_request_info*);


bool init_request_handler ();

void register_request (reqest_t*);

#endif // __REQUEST_HANDLER_H

