#ifndef FCGI_H
#define FCGI_H

#include <stddef.h>

#define MAX_REQUESTS 10

struct fcgi_connection
{
    int fd;
    int request_id;
    int role;
    char flags;
    int stdin;
    int stdout;
    int stderr;
    char** env;
};

enum fcgi_protocol_status
{
    REQUEST_COMPLETE = 0,
    CANT_MPX_CONN = 1,
    OVERLOADED = 2,
    UNKNOWN_ROLE = 3
};


void fcgi_init ();
struct fcgi_connection* fcgi_loop (int fd);
int fcgi_end (struct fcgi_connection* conn, enum fcgi_protocol_status status, 
        int app_status);

#endif //FCGI_H

