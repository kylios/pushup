#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <event.h>


#include "debug.h"
#include "networking.h"

/**
 * Function taken from 
 * https://www.ibm.com/developerworks/aix/library/au-libev/
 * */
int
setnonblock (int fd)
{
    int flags;
    flags = fcntl (fd, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl (fd, F_SETFL, flags);
};

static int
get_addr_type (const char* addr)
{
    if (NULL != strchr (addr, ':'))
    {
        return 6;
    }
    else if (NULL != strchr (addr, '.'))
    {
        return 4;
    }
    else
    {
        return -1;
    }
};

int 
bind_addr_port (const char* addr, int ipver, const char* port)
{
    ASSERT (addr != NULL);
    ASSERT (port != NULL);
    ASSERT (ipver == 4 || ipver == 6);

    struct addrinfo hints;
    struct addrinfo* servinfo;
    struct addrinfo* p;

    memset (&hints, 0, sizeof hints);
    hints.ai_family = (ipver == 4 ? AF_INET : AF_INET6);
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int rv;
    if ((rv = getaddrinfo (NULL, port, &hints, &servinfo)) != 0)
    {
        printf ("getaddrinfo: %s", gai_strerror (rv));
        return -1;
    }

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
        printf ("scanning %s: %s... \n", ipver_str, ipstr);

        if ((fd = socket (p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
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
            close (fd);
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

    freeaddrinfo (servinfo);
    
    return fd;
};

int 
bind_unix_socket (const char* path)
{
    ASSERT (path != NULL);

    unsigned int s, s2;
    struct sockaddr_un local, remote;
    int len;

    if (-1 == (s = socket (AF_UNIX, SOCK_STREAM, 0)))
    {
        printf ("failed to create unix socket\n");
        return -1;
    }

    local.sun_family = AF_UNIX;
    strcpy (local.sun_path, path);
    unlink (local.sun_path);
    len = strlen (local.sun_path) + sizeof (local.sun_family);
    bind (s, (struct sockaddr*) &local, len);

    return s;
};

