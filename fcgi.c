#include <shm.h>
#include <stdio.h>
#include <stdlib.h>

#include "fcgi.h"

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

enum fcgi_protocol_status
{
    REQUEST_COMPLETE = 0,
    CANT_MPX_CONN = 1,
    OVERLOADED = 2,
    UNKNOWN_ROLE = 3
};

struct fcgi_header
{
    int version;
    enum fcgi_type type;
    int request_id;
    size_t content_length;
    size_t padding_length;
};

struct literal_fcgi_header
{
    unsigned char version;
    unsigned char type;
    unsigned char request_id_1;
    unsigned char request_id_0;
    unsigned char content_length_1;
    unsigned char content_length_0;
    unsigned char padding_length;
    unsigned char reserved;
};

struct fcgi_begin_request_body 
{
    unsigned char role_1;
    unsigned char role_0;
    unsigned char flags;
    unsigned char reserved[5];
};

struct fcgi_end_request_body
{
    unsigned char app_status_3;
    unsigned char app_status_2;
    unsigned char app_status_1;
    unsigned char app_status_0;
    unsigned char protocol_status;
    unsigned char reserved;
};

typedef int fcgi_record_processing_func (struct fcgi_header*, 
        struct fcgi_connection*, const char* buf);

struct fcgi_header* tmp_XheaderX_dontuse;
#define FCGI_SEND(__tmp_XheaderX_dontuse, fd, _version, _request_id, _type, sz, data) \
    tmp_XheaderX_dontuse = (__tmp_XheaderX_dontuse); \
    memset ((tmp_XheaderX_dontuse), 0, (sizeof (struct fcgi_header))); \
    tmp_XheaderX_dontuse->version = (_version);  \
    tmp_XheaderX_dontuse->request_id = (_request_id);  \
    tmp_XheaderX_dontuse->type = (_type);  \
    tmp_XheaderX_dontuse->content_length = (sz);  \
    tmp_XheaderX_dontuse->padding_length = 0;   \
    fcgi_send_packet ((fd), tmp_XheaderX_dontuse, (const char*) (data)); 
    
int fcgi_send_packet (int fd, struct fcgi_header* header, const char* buf);

int fcgi_process_begin_request (struct fcgi_header*, struct fcgi_connection*, 
        const char* buf);
int fcgi_process_abort_request (struct fcgi_header*, struct fcgi_connection*, 
        const char* buf);
int fcgi_process_end_request (struct fcgi_header*, struct fcgi_connection*, 
        const char* buf);
int fcgi_process_params (struct fcgi_header*, struct fcgi_connection*, 
        const char* buf);
int fcgi_process_stdin (struct fcgi_header*, struct fcgi_connection*, 
        const char* buf);
int fcgi_process_stdout (struct fcgi_header*, struct fcgi_connection*, 
        const char* buf);
int fcgi_process_stderr (struct fcgi_header*, struct fcgi_connection*, 
        const char* buf);
int fcgi_process_data (struct fcgi_header*, struct fcgi_connection*, 
        const char* buf);
int fcgi_process_get_values (struct fcgi_header*, struct fcgi_connection*,
        const char* buf);
int fcgi_process_get_values_result (struct fcgi_header*, struct fcgi_connection*, 
        const char* buf);
int fcgi_process_unknown_type (struct fcgi_header*, struct fcgi_connection*, 
        const char* buf);


/*
 * Use this to keep track of all the connections
 * */
static struct fcgi_connection* connections[MAX_REQUESTS];


static int fcgi_read_header (int fd, struct fcgi_header* header);
static void fcgi_dump_header (struct fcgi_header* header);



static const char* send_str = 
"Content-Type: text/html \r\n"
"\r\n"
"hello";

static fcgi_record_processing_func *fcgi_handlers[12];

struct fcgi_connection* fcgi_lookup_connection (int request_id);

void
fcgi_init ()
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

    // Clear the connections table
    memset (connections, 0, sizeof (struct fcgi_connection*) * MAX_REQUESTS);

    // TODO: init the data structures and threads necessary for keeping track of
    // each request
};

struct fcgi_connection*
fcgi_loop (int fd)
{
    struct fcgi_connection* conn;

    enum fcgi_type last_request = UNKNOWN_TYPE;
    while (last_request != STDIN)
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
            fcgi_init_connection (conn);
            conn->fd = fd;
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
        last_request = header.type;
    }
    return 1;
};

void
fcgi_init_connection (struct fcgi_connection* conn)
{
    memset (conn, 0, sizeof (*conn));
    conn->stdin = shm_open ("stdin", O_RDONLY | O_CREAT, S_IRUSR | S_IRGRP);
    conn->stdout = shm_open ("stdout", O_RDONLY | O_CREAT, S_IRUSR | S_IRGRP);
    conn->stderr = shm_open ("stderr", O_RDONLY | O_CREAT, S_IRUSR | S_IRGRP);
};

int
fcgi_write (struct fcgi_connection* conn, size_t sz, const char* buf)
{

    struct fcgi_header x;
    FCGI_SEND(&x, fd, 1, 1, STDOUT, strlen (send_str), send_str);
};

int
fcgi_end (struct fcgi_connection* conn, enum fcgi_protocol_status, int app_status)
{
    CHECK_PTR(conn);

    struct fcgi_end_request_body end;
    end.protocol_status = REQUEST_COMPLETE;
    end.app_status_3 = (unsigned char) (app_status & 0xff000000) >> 24;
    end.app_status_2 = (unsigned char) (app_status & 0x00ff0000) >> 16;
    end.app_status_1 = (unsigned char) (app_status & 0x0000ff00) >> 8;
    end.app_status_0 = (unsigned char) app_status & 0x000000ff;

    struct fcgi_header x;
    FCGI_SEND (&x, fd, 1, 1, END_REQUEST, sizeof (struct fcgi_end_request_body), (const char*) &end);

    close (conn->fd);    
};

int 
fcgi_read_header (int fd, struct fcgi_header* header)
{
    printf ("reading header...\n");
    struct literal_fcgi_header h;
    if (!recv (fd, &h, sizeof (h), 0))
    {
        return -1;
    }
    printf ("done reading header \n");

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

inline int 
fcgi_send_packet (int fd, struct fcgi_header* header, const char* buf)
{
    if (!header)
    {
        return 0;
    }

    struct literal_fcgi_header h;
    h.version = (unsigned char) header->version;
    h.type = (unsigned char) header->type;
    h.request_id_1 = (unsigned char) (header->request_id >> 8);
    h.request_id_0 = (unsigned char) (header->request_id);
    h.content_length_1 = (unsigned char) (header->content_length >> 8);
    h.content_length_0 = (unsigned char) header->content_length;
    h.padding_length = (unsigned char) header->padding_length;

    printf ("version: %x \n", h.version);
    printf ("type: %x \n", h.type);
    printf ("request_id_1: %x \n", h.request_id_1);
    printf ("request_id_0: %x \n", h.request_id_0);
    printf ("content_length_1: %x \n", h.content_length_1);
    printf ("content_length_0: %x \n", h.content_length_0);
    printf ("padding_length: %x \n", h.padding_length);
    
    send (fd, &h, sizeof (struct literal_fcgi_header), 0);
    send (fd, buf, header->content_length, 0);
    
    return 1;
};

#define CHECK_PTR(ptr) \
    if (!(ptr)) \
    { \
        return 0; \
    }

int 
fcgi_process_begin_request (struct fcgi_header* header, 
        struct fcgi_connection* conn, const char* buf)
{
    CHECK_PTR(header);
    CHECK_PTR(conn);
    CHECK_PTR(buf);

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

/*
 * Probably not called ever.
 * */
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
    const char* _buf = buf;

    printf ("Length: %d \n", header->content_length);
    while (_buf + header->content_length > buf)
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

        printf ("Name Data: %s \n", name_data);
        printf ("Value Data: %s \n", value_data);
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
    return connections[request_id - 1];    
};







