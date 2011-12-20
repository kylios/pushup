#include <stdlib.h>
#include <event.h>

#include "type.h"
#include "debug.h"

#include "event.h"
#include "fcgi.h"

static void buf_read_callback (struct bufferevent* incoming, void* arg);
static void buf_write_callback (struct bufferevent* bev, void* arg);
static void buf_error_callback (struct bufferevent* bev, short what, void* arg);

void 
accept_callback (int fd, short ev, void* arg)
{
    int client_fd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof client_addr;
    struct fcgi* client;

    client_fd = accept (fd, (struct sockaddr*) &client_addr, &client_len);
    if (client_fd < 0)
    {
        return;
    }

    setnonblock (client_fd);

    client = calloc (1, sizeof *client);
    if (client == NULL)
    {
        return;
    }

    client->sockfd = client_fd;
    client->bufev = bufferevent_new (client_fd, 
            buf_read_callback,
            buf_write_callback,
            buf_error_callback,
            (void*) client);
    bufferevent_enable (client->bufev, EV_READ);
};

static void 
buf_read_callback (struct bufferevent* in, void* arg)
{
    struct evbuffer *evbuf;
    char* req;
    size_t num;

    printf ("a\n");
    req = (char*) malloc (sizeof (struct frame_header));
    if (NULL == req)
    {
        return;
    }
    
    while (sizeof (struct frame_header) == 
            (num = bufferevent_read (in, req, sizeof (struct frame_header))))
    {
        printf ("b\n");
        if (0 == fcgi_read (req, num, in))
        {
            // Error
            return;
        }
        printf ("c\n");
    }
//
//    evbuf = evbuffer_new ();
//    evbuffer_add_printf (evbuf, "You said %s \n", req);
//    bufferevent_write_buffer (in, evbuf);
//    evbuffer_free (evbuf);
//    free (req);
};

static void 
buf_write_callback (struct bufferevent* bev, void* arg)
{
    
};

static void 
buf_error_callback (struct bufferevent* bev, short what, void* arg)
{
    struct fcgi* client;
    bufferevent_free (client->bufev);
    close (client->sockfd);
    free (client);
};


