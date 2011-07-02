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

void fcgi_init ();
struct fcgi_connection* fcgi_loop (int fd);

#endif //FCGI_H

