#include "fcgi.h"

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

static const char* send_str = 
"Content-Type: text/html \r\n"
"\r\n"
"hello";

static fcgi_record_processing_func *fcgi_handlers[12];

int bind_to_localhost (const char* port, int ipver, int* sockfd);

struct fcgi_connection* fcgi_lookup_connection (int request_id);

int
main (int argc, char** argv)
{
    fcgi_handlers[BEGIN_REQUEST] = fcgi_process_begin_request;
    fcgi_handlers[ABORT_REQUEST] = fcgi_process_abort_request;
    fcgi_handlers[END_REQUEST] = fcgi_process_end_request;
    fcgi_handlers[PARAMS] = &fcgi_process_params;
    fcgi_handlers[STDIN] = &fcgi_process_stdin;
    fcgi_handlers[STDOUT] = &fcgi_process_stdout;
    fcgi_handlers[STDERR] = &fcgi_process_stderr;
    fcgi_handlers[DATA] = &fcgi_process_data;
    fcgi_handlers[GET_VALUES] = &fcgi_process_get_values;
    fcgi_handlers[GET_VALUES_RESULT] = &fcgi_process_get_values_result;
    fcgi_handlers[UNKNOWN_TYPE] = &fcgi_process_unknown_type;

    struct sockaddr_storage client_addr;
    socklen_t sin_size;
    struct sockaddr_storage their_addr;
    int sockfd;
    const char* port = "13337";

    if (-1 == bind_to_localhost (port, 4, &sockfd))
    {
        printf ("FAIL\n");
    }
    if (-1 == listen (sockfd, 5))
    {
        printf ("FAIL2\n");
    }

    int run = 1;
    while (run)
    {
        int fd = accept (sockfd, (struct sockaddr*) &their_addr, &sin_size);
        if (-1 == fd)
        {
            printf ("CONNECTION FAIL \n");
            continue;
        }
        else
        {
            printf ("connected \n");
            while (1)
            {
                struct fcgi_header header;
                if (-1 == fcgi_read_header (fd, &header))
                {
                    break;
                }
                fcgi_dump_header (&header);
                struct fcgi_connection* conn = 
                    fcgi_lookup_connection (header.request_id);
                if (!conn)
                {
                    // Allocate a new connection object
                    conn = (struct fcgi_connection*) 
                        malloc (sizeof (struct fcgi_connection));
                }
                if (header.content_length > 0) 
                {
                    // Allocate a buffer to hold the Content and padding
                    char* str = (char*) malloc (sizeof (char) * 
                            (header.content_length + header.padding_length));
                    if (!str) 
                    {
                        break;
                    }
                    if (!recv (fd, str, 
                            header.content_length + header.padding_length, 0))
                    {
                        free(str);
                        break;
                    }

                    // Call the respective handler function for the type of
                    // packet.
                    if (!fcgi_handlers[header.type] 
                            (&header, conn, str))
                    {
                        free (str);
                        break;
                    }

                    free (str);
                }
            }
            send(fd, send_str, strlen(send_str), 0);
            printf ("connection over \n");
        }
    }
};

int 
fcgi_read_header (int fd, struct fcgi_header* header)
{
    struct literal_fcgi_header h;
    if (!recv (fd, &h, sizeof (h), 0))
    {
        return -1;
    }

    header->version = (int) h.version;
    header->type = (enum fcgi_type) h.type;
    header->request_id = (int) (h.request_id_1 << 8) + (int) h.request_id_0;
    header->content_length = (size_t)
        ((int) (h.content_length_1 << 8) + (int) h.content_length_0);
    header->padding_length = (size_t) ((int) h.padding_length);

    return 0;
};


void 
fcgi_dump_header (struct fcgi_header* header)
{
    printf ("version: %d \n", header->version);
    printf ("type: %d \n", header->type);
    printf ("request_id: %d \n", header->request_id);
    printf ("content_length: %d \n", header->content_length);
    printf ("padding_length: %d \n", header->padding_length);
};

int 
fcgi_process_begin_request (struct fcgi_header* header, 
        struct fcgi_connection* conn, const char* buf)
{
    if (!conn)
    {
        return 0;
    }
    struct fcgi_begin_request_body* b = 
        (struct fcgi_begin_request_body*) buf;

    conn->role = (int) (b->role_1 << 8) + (int) b->role_0;
    conn->flags = b->flags;
    printf ("Role: %d \n", conn->role);
    printf ("Flags: %x \n", conn->flags);

    return 1;     
};

int 
fcgi_process_abort_request (struct fcgi_header* header, 
        struct fcgi_connection* conn, const char* buf)
{
    printf ("abort request \n");
    return 0;    
};

int 
fcgi_process_end_request (struct fcgi_header* header, 
        struct fcgi_connection* conn, const char* buf)
{
    printf ("end request \n");
    return 0;   
};

int 
fcgi_process_params (struct fcgi_header* header, 
        struct fcgi_connection* conn, const char* buf)
{
    size_t sz = 0;

    printf ("Length: %d \n", header->content_length);
    while (sz < header->content_length)
    {
        unsigned char name_length_0 = *buf++;
//        printf ("NameLength0: %x \n", name_length_0);
        int name_length = (int) name_length_0;
        if (name_length_0 >> 7) // High-order bit is one
        {
            unsigned char   name_length_1 = *buf++,
                            name_length_2 = *buf++,
                            name_length_3 = *buf++;
//            printf ("NameLength1: %x \n", name_length_1);
//            printf ("NameLength2: %x \n", name_length_2);
//            printf ("NameLength3: %x \n", name_length_3);
            name_length = 
                ((int) (name_length_3 & 0x7f) << 24) + 
                (int) (name_length_2 << 16) + 
                (int) (name_length_1 << 8) + 
                (int) name_length_0;
        }
        unsigned char value_length_0 = *buf++;
//        printf ("ValueLength0: %x \n", value_length_0);
        int value_length = (int) value_length_0;
        if (value_length_0 >> 7)
        {
            unsigned char value_length_1 = *buf++, 
                          value_length_2 = *buf++, 
                          value_length_3 = *buf++;
//            printf ("ValueLength1: %x \n", value_length_1);
//            printf ("ValueLength2: %x \n", value_length_2);
//            printf ("ValueLength3: %x \n", value_length_3);
            value_length = 
                ((int) (value_length_3 & 0x7f) << 24) + 
                (int) (value_length_2 << 16) + 
                (int) (value_length_1 << 8) + 
                (int) value_length_0;
        }

        if (sz + name_length + value_length > header->content_length)
        {
            break;
        }

        char* name_data = (char*) malloc (sizeof (char) * name_length + 1);
        if (!name_data)
        {
            return 0;
        }

        char* value_data = (char*) malloc (sizeof (char) * value_length + 1);
        if (!value_data)
        {
            free (name_data);
            return 0;
        }

        memcpy (name_data, buf, name_length);
        name_data[name_length] = '\0';
        buf += name_length;
        memcpy (value_data, buf, value_length);
        value_data[value_length] = '\0';
        buf += value_length;

//        printf ("Name Data: %s \n", name_data);
//        printf ("Value Data: %s \n", value_data);

        sz += name_length + value_length;
        printf ("sz: %d \n", sz);
    }

    return 1;
};

int 
fcgi_process_stdin (struct fcgi_header* header, 
        struct fcgi_connection* conn, const char* buf)
{
    char* stdin = (char*) malloc (sizeof (char) * header->content_length);
    if (!stdin)
    {
        return 0;
    }

    memcpy (stdin, buf, header->content_length);
    printf ("stdin: %s \n", stdin);
    return 1;
};

int 
fcgi_process_stdout (struct fcgi_header* header, 
        struct fcgi_connection* conn, const char* buf)
{
    printf ("stdout \n");
    return 0;
};

int 
fcgi_process_stderr (struct fcgi_header* header, 
        struct fcgi_connection* conn, const char* buf)
{
    printf ("stderr \n");
    return 0;
};

int 
fcgi_process_data (struct fcgi_header* header, 
        struct fcgi_connection* conn, const char* buf)
{
    printf ("data \n");
    return 0;
};

int 
fcgi_process_get_values (struct fcgi_header* header, 
        struct fcgi_connection* conn, const char* buf)
{
    printf ("get values \n");
    return 0;
};

int 
fcgi_process_get_values_result (struct fcgi_header* header, 
        struct fcgi_connection* conn, const char* buf)
{
    printf ("get values result \n");
    return 0;
};

int 
fcgi_process_unknown_type (struct fcgi_header* header, 
        struct fcgi_connection* conn, const char* buf)
{
    printf ("unknown type \n");
    return 0;
};

struct fcgi_connection* 
fcgi_lookup_connection (int request_id)
{
    return NULL;    
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
