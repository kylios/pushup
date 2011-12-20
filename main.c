#include <event.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "type.h"
#include "debug.h"

#include "main.h"
#include "fcgi.h"
#include "networking.h"
#include "event.h"


/*

Initialize libevent to run callbacks when our socket has new data to read.  
The callback should fork a new process which will begin reading fcgi packets.
Our fcgi sessions are stored in the structure defined in fcgi.c, and once The
request_id is determined from the fcgi packet, the fcgi session structure is
retrieved and reading/processing continues on that packet.
*/

int 
main (int argc, char** argv)
{
    event_init ();
    init_fcgi ();

//    int listen_fd = bind_unix_socket (SOCKET_PATH);
    int listen_fd = bind_addr_port ("127.0.0.1", 4, SERVER_PORT);

    if (-1 == listen (listen_fd, 0))
    {
        printf ("Failed to listen \n");
        return 1;
    }

    int reuse = 1;
    struct event accept_event;
    setsockopt (listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof reuse);
    setnonblock (listen_fd);
    event_set (&accept_event, listen_fd, EV_READ | EV_PERSIST, accept_callback, NULL);
    event_add (&accept_event, NULL);
    printf ("dispatching... \n");
    event_dispatch ();
    printf ("FIN. \n");
    close (listen_fd);
    return 0;
};

