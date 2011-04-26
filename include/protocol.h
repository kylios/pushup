#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "type.h"
#include "lib/mongoose.h"

/**
 * This file defines our protocol for pushing and receiving events.
 * */


/*
 * Parameters that can be passed to the server
 * */
#define PROTO_ACTION a
#define PROTO_USER   u
#define PROTO_SESS   s
#define PROTO_MESS   m

typedef enum {
      PR_PUSH   // pushing an event to the server
    , PR_UPDATE // requesting an update for events in the queue
    , PR_REG    // register (login) a user with a session
    , PR_UREG   // unregister (logout) a user with a session
} reqtype_t;

typedef struct {
    reqtype_t reqtype
    , char* a
    , char* u
    , char* s
    , char* m
} protocol_info_t;

/**
 * Evaluates the request and parses all the information out of it, including the
 * request type and parameters, etc. 
 * */
reqtype_t protocol_eval (protocol_info_t*, mg_request_info*);


#endif // PROTOCOL_H
