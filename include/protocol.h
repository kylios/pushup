#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string.h> 

#include "type.h"
#include "lib/mongoose.h"

/**
 * This file defines our protocol for pushing and receiving events.
 * */


/*
 * Parameters that can be passed to the server
 * */
#define PROTO_ACTION    "a"
#define PROTO_USER      "u"
#define PROTO_SESSION   "s"
#define PROTO_MESSAGE   "m"
#define PROTO_DEBUG     "x"

/*
 * Define the maximum sizes of each of these request parameters.
 * */
#define ACTION_STR_SZ   1               // Should always be 1 char code
#define USER_STR_SZ     32              // designed for md5 hash
#define SESSION_STR_SZ  32              // designed for md5 hash
#define MESSAGE_STR_SZ  10*1024*1024    // 10 MB max message size
#define DEBUG_STR_SZ    1024            

typedef enum reqtype {
      PR_REG = 0    // register (login) a user with a session
    , PR_UREG = 1   // unregister (logout) a user with a session
    , PR_PUSH = 2   // pushing an event to the server
    , PR_UPDATE = 3 // requesting an update for events in the queue
    , PR_DEBUG = 4  // For testing only
    , PR_DEFAULT = 5// default (error) type
} reqtype_t;

typedef struct {
    reqtype_t reqtype;
    char* a;    // action
    char* u;    // user
    char* s;    // session
    char* m;    // message
    char* x;    // debug
} protocol_info_t;

#define require(PARAM){ \
    if ((PARAM) == NULL || strlen((PARAM)) == 0){ \
        return PR_DEFAULT; \
    } \
}

/**
 * Evaluates the request and parses all the information out of it, including the
 * request type and parameters, etc. 
 * */
reqtype_t protocol_eval (protocol_info_t*, const char*, 
        const struct mg_request_info*);


#endif // PROTOCOL_H
