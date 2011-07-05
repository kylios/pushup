#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "test.h"
#include "fcgi.h"


static const char* send_str = 
"Content-Type: text/html \r\n"
"\r\n"
"hello";

int 
main (int argc, char** argv)
{
    struct sockaddr_storage client_addr;
    socklen_t sin_size;
    struct sockaddr_storage their_addr;
    int sockfd;
    const char* port = "8082";

    fcgi_init ();

    if (-1 == bind_to_localhost (port, 4, &sockfd))
    {
        printf ("FAIL\n");
    }
    if (-1 == listen (sockfd, 5))
    {
        printf ("FAIL2\n");
    }

    while (1)
    {
        int fd = accept (sockfd, (struct sockaddr*) &their_addr, &sin_size);
        if (-1 == fd)
        {
            printf ("CONNECTION FAIL \n");
            continue;
        }
        else
        {
            struct fcgi_connection* request = fcgi_loop (fd);
            if (request == NULL)
            {
                break;
            }
            printf ("Loop finished: \n");
            printf ("request_id: %d \n", request->request_id);
            printf ("role: %d \n", request->role);
            printf ("flags: %c \n", request->flags);

            printf ("Calling fcgi_write...\n");
            fcgi_write (request, send_str, strlen (send_str));

            printf ("Calling fcgi_end...\n");
            if (0 == fcgi_end (request, REQUEST_COMPLETE, 0))
            {
                printf ("failed...");
            }
            printf ("DONE\n");

        }
    }

};



int
bind_to_localhost (const char* port, int ipver, int* sockfd)
{
    struct addrinfo hints;
    struct addrinfo* servinfo;
    struct addrinfo* p;

    /* Set up the addrinfo struct to
    * * -Use ipv4 or ipv6, whichever was specified
    * * -Use a stream socket
    * * -Use passive mode
    * * */
    memset (&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;//(ipver == 4 ? AF_INET : AF_INET6);
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int rv;
    if ((rv = getaddrinfo (NULL, port, &hints, &servinfo)) != 0)
    {
        printf ("getaddrinfo: %s", gai_strerror (rv));
        return -1;
    }

    /* Find an address to bind to. Since we just need to bind to ourselves,
    * * this should not fail.
    * * */
    int yes = 1;
    int fd;

    char ipstr[INET6_ADDRSTRLEN];
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        char* ipver_str;
        void* addr;

        if (p->ai_family == AF_INET)
        {
            struct sockaddr_in* ipv4 = (struct sockaddr_in*) p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver_str = "IPv4";
        }
        else
        {
            struct sockaddr_in6* ipv6 = (struct sockaddr_in6*) p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver_str = "IPv6";
        }

        inet_ntop (p->ai_family, addr, ipstr, sizeof ipstr);
        printf ("scanning %s: %s...\n", ipver_str, ipstr);

        if ((fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
        {
            printf ("socket");
            continue;
        }

        if (setsockopt (fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (int)) == -1) 
        {
            printf ("setsockopt");
            close (fd);
            return -1;
        }
           
        if (bind (fd, p->ai_addr, p->ai_addrlen) == -1) 
        {
            close(fd);
            printf ("bind");
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        printf ("failed to bind");
        return -1;
    }

    freeaddrinfo(servinfo); // all done with this structure

    /* This is the socket we'll return to the caller */
    *sockfd = fd;

    return 0;
}
