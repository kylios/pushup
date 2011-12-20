#ifndef __FCGI_H
#define __FCGI_H

#include <stddef.h>
#include <event.h>

#include "trie.h"

struct fcgi
{
    int request_id;
    int version;
    int sockfd; 
    int role;
    struct bufferevent* bufev;
    char flags;

    /* key => value mapping for environment variables */
    struct trie env;

    char* _stdin;
};

enum fcgi_type
{
    BEGIN_REQUEST = 1,
    ABORT_REQUEST = 2,
    END_REQUEST = 3,
    PARAMS = 4,
    STDIN = 5,
    STDOUT = 6,
    STDERR = 7,
    DATA = 8,
    GET_VALUES = 9,
    GET_VALUES_RESULT = 10,
    UNKNOWN_TYPE = 11
};

enum fcgi_role
{
    FCGI_RESPONDER = 1,
    FCGI_AUTHORIZER = 2,
    FCGI_FILTER = 3
};

/*
 * Literal layout of a fastcgi multiplexed frame.
 * */
struct frame_header
{
    byte version;
    byte type;
    byte request_id_1;
    byte request_id_0;
    byte content_length_1;
    byte content_length_0;
    byte padding_length;
    byte reserved;
};
void init_fcgi ();
int fcgi_read (const char* str, size_t num, struct bufferevent* in);

void fcgi_env_add (struct fcgi* fcgi, const char* key, const char* val);
const char* fcgi_env_get (struct fcgi* fcgi, const char* key);

#endif //__FCGI_H

