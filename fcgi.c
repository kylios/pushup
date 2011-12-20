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

#include "fcgi.h"
#include "networking.h"
#include "bst.h"
#include "trie.h"

/*
 * The following structs represent literal memory layouts for incoming frames.
 * These will be used to parse information out of the requests.
 * */

struct begin_request_body
{
    byte role_1;
    byte role_0;
    byte flags;
    byte reserved[5];
};
struct end_request_body
{
    byte app_status_3;
    byte app_status_2;
    byte app_status_1;
    byte app_status_0;
    byte protocol_status;
    byte reserved[3];
};
struct name_len_1
{
    byte name_length_0;
};
struct name_len_4
{
    byte name_length_3;
    byte name_length_2;
    byte name_length_1;
    byte name_length_0;
};
struct value_len_1
{
    byte value_length_0;
};
struct value_len_4
{
    byte value_length_3;
    byte value_length_2;
    byte value_length_1;
    byte value_length_0;
};


/*
 * Data structures to keep track of fast cgi sessions, or connections.  These
 * objects will be stored in an index structure due to fastcgi's multiplexing,
 * we will need to access data from different sessions intermittently.
 * */
struct frame
{
    enum fcgi_type type;
    int request_id;
    int version;
    size_t content_length;
    size_t padding_length;
    char* content;
    char* padding;
};

typedef int process_frame_func (struct frame* f, struct fcgi* fcgi, const char* buf, size_t num);

/*
 * This structure keeps track of all our fcgi sessions.  Since fcgi is a
 * multiplexing protocol, we could receive frames from different requests before
 * any of the requests have completed.  Therefore we need to keep track of all
 * open requests so that we can apply the incoming frame to its associated session.
 * Also includes a lock for multithreading.
 * */
static struct bst_tree _fcgi_sessions;
static pthread_mutex_t _fcgi_sessions_lock;
static struct bst_tree* fcgi_sessions = &_fcgi_sessions;
static pthread_mutex_t* fcgi_sessions_lock = &_fcgi_sessions_lock;

static process_frame_func *handlers[12];

static void fcgi_init (struct fcgi*, struct frame*);
static struct fcgi* lookup (struct frame* f);

static int compare_sessions (const void* a, const void* b, const void* AUX);

static int process_begin_request (struct frame*, struct fcgi*, const char*, size_t num);
static int process_end_request (struct frame*, struct fcgi*, const char*, size_t num);
static int process_abort_request (struct frame*, struct fcgi*, const char*, size_t num);
static int process_params (struct frame*, struct fcgi*, const char*, size_t num);
static int process_stdin (struct frame*, struct fcgi*, const char*, size_t num);
static int process_stdout (struct frame*, struct fcgi*, const char*, size_t num);
static int process_stderr (struct frame*, struct fcgi*, const char*, size_t num);
static int process_data (struct frame*, struct fcgi*, const char*, size_t num);
static int process_get_values (struct frame*, struct fcgi*, const char*, size_t num);
static int process_get_values_result (struct frame*, struct fcgi*, const char*, size_t num);
static int process_unknown_type (struct frame*, struct fcgi*, const char*, size_t num);

void
init_fcgi ()
{
    bst_init (fcgi_sessions, compare_sessions);
    pthread_mutex_init (fcgi_sessions_lock, NULL);

    handlers[BEGIN_REQUEST] = process_begin_request;
    handlers[ABORT_REQUEST] = process_abort_request;
    handlers[END_REQUEST] = process_end_request;
    handlers[PARAMS] = process_params;
    handlers[STDIN] = process_stdin;
    handlers[STDOUT] = process_stdout;
    handlers[STDERR] = process_stderr;
    handlers[DATA] = process_data;
    handlers[GET_VALUES] = process_get_values;
    handlers[GET_VALUES_RESULT] = process_get_values_result;
    handlers[UNKNOWN_TYPE] = process_unknown_type;
};

int
fcgi_read (const char* data, size_t num, struct bufferevent* in)
{
    struct frame_header header;
    struct frame f;
    struct fcgi* fcgi;

    ASSERT (data);

    printf ("reading new frame \n");

    // TODO: should we implement some sort of protection here?  there should be
    // at least enough bytes in data to fill a frame_header struct, but how can
    // we be sure?
    memcpy ((void*) &header, (void*) data, sizeof header);

    f.type = (enum fcgi_type) header.type;
    f.request_id = (int) (header.request_id_1 << 8) + (int) header.request_id_0;
    f.version = (int) header.version;
    f.content_length = (size_t)
        ((int) (header.content_length_1 << 8) + (int) header.content_length_0);
    f.padding_length = (size_t) ((int) header.padding_length);

    printf ("    type=%d, request_id=%d, content_length=%u, padding_length=%u \n",
            f.type, f.request_id, f.content_length, f.padding_length);

    /*
     * Check if the web server actually supplied enough data to fill
     * content_data and padding_data.  If it didn't, that means the webserver or
     * client crashed and the connection was severed.
     * */
    if (num <= f.content_length + f.padding_length)
    {
        // TODO: set error code
        return -1;
    }

    fcgi = lookup (&f);
    if (NULL == fcgi)
    {
        printf ("Could not find session.  Creating new one \n");
        fcgi = (struct fcgi*) malloc (sizeof (struct fcgi));
        if (NULL == fcgi)
        {
            // TODO: set error code
            return -1;
        }

        fcgi_init (fcgi, &f);
        bst_insert (fcgi_sessions, fcgi);
        printf ("created new session! \n");
    }

    f.content = (char*) malloc (sizeof (char) * f.content_length);
    if (NULL == f.content)
    {
        printf ("    content_length: %d \n", f.content_length);
        // TODO: set error code
        return -1;
    }
    f.padding = (char*) malloc (sizeof (char) * f.padding_length);
    if (NULL == f.padding)
    {
        printf ("    padding_length: %d \n", f.padding_length);
        free (f.content);
        // TODO: set error code
        return -1;
    }
    if (f.content_length != 
            bufferevent_read (in, f.content, f.content_length) ||
        f.padding_length != 
            bufferevent_read (in, f.padding, f.padding_length))
    {
        printf ("a \n");
        free (f.content);
        free (f.padding);
        // TODO: set error code
        return -1;
    }

    printf ("reading content...\n");
    if (!handlers[f.type] (&f, fcgi, f.content, num))
    {
        printf ("ahhhh! \n");
        // TODO: set error code
        return 0;
    }

    printf ("returning... \n");
    return 1;
};   

static void
fcgi_init (struct fcgi* fcgi, struct frame* f)
{
    ASSERT (fcgi);
    ASSERT (f);

    fcgi->request_id = f->request_id;
    fcgi->version = f->version;

    trie_init (&fcgi->env);
};

void
fcgi_env_add (struct fcgi* fcgi, const char* key, const char* val)
{
    ASSERT (fcgi);
    ASSERT (key);
    ASSERT (val);

    trie_insert (&fcgi->env, key, val);
};

const char*
fcgi_env_get (struct fcgi* fcgi, const char* key)
{
    ASSERT (fcgi);
    ASSERT (key);

    return (const char*) trie_find (&fcgi->env, key);
};

static struct fcgi*
lookup (struct frame* f)
{
    struct fcgi _f;

    ASSERT (f);

    _f.request_id = f->request_id;

    return bst_find (fcgi_sessions, &_f);
};

static int 
compare_sessions (const void* _a, const void* _b, const void* AUX)
{
    const struct fcgi* a;
    const struct fcgi* b;

    ASSERT (_a);
    ASSERT (_b);

    a = (const struct fcgi*) _a;
    b = (const struct fcgi*) _b;

    return a->request_id - b->request_id;
};

static int 
process_begin_request (struct frame* f, struct fcgi* fcgi, const char* str, size_t num)
{
    struct begin_request_body* b;

    ASSERT (f);
    ASSERT (fcgi);
    ASSERT (str);
        
    b = (struct begin_request_body*) str;

    fcgi->role = (int) (b->role_1 << 8) + (int) b->role_0;
    fcgi->flags = b->flags;

    printf ("BEGIN REQUEST: role=%d, flags=%d \n", fcgi->role, fcgi->flags);

    return 1;
};

static int
process_abort_request (struct frame* f, struct fcgi* fcgi, const char* str, size_t num)
{
    return 1;
};

static int 
process_end_request (struct frame* f, struct fcgi* fcgi, const char* str, size_t num)
{
    return 1;
};

static int
process_params (struct frame* f, struct fcgi* fcgi, const char* _str, size_t num)
{
    const char* str;

    ASSERT (_str);

    printf ("PARAMS: %s \n", _str);

    str = _str;
    size_t name_len = 0;
    size_t value_len = 0;
    size_t accum = 0;

    while (accum < f->content_length)
    {
        if (*((byte*) str) >> 7) // High-order bit is one
        {
            struct name_len_4 nlen;
            memcpy (&nlen, str, sizeof (nlen));
            str += sizeof (nlen);

            name_len = (int) nlen.name_length_0 + 
                        (int) (nlen.name_length_1 << 8) +
                        (int) (nlen.name_length_2 << 16) + 
                        (int) ((nlen.name_length_3 & 0x7f) << 24);
            accum += sizeof (nlen) + name_len;
        }
        else
        {
            struct name_len_1 nlen;
            memcpy (&nlen, str, sizeof (nlen));
            str += sizeof (nlen);

            name_len = (int) nlen.name_length_0;
            accum += sizeof (nlen) + name_len;
        }

        if (*((byte*) str) >> 7)
        {
            struct value_len_4 vlen;
            memcpy (&vlen, str, sizeof (vlen));
            str += sizeof (vlen);

            value_len = (int) vlen.value_length_0 + 
                        (int) (vlen.value_length_1 << 8) +
                        (int) (vlen.value_length_2 << 16) +
                        (int) ((vlen.value_length_3 & 0x7f) << 24);
            accum += sizeof (vlen) + value_len;
        }
        else
        {
            struct value_len_1 vlen;
            memcpy (&vlen, str, sizeof (vlen));
            str += sizeof (vlen);

            value_len = (int) vlen.value_length_0;
            accum += sizeof (vlen) + value_len;
        }

        char* name_data = (char*) malloc (sizeof (char) * (name_len + 1));
        if (NULL == name_data)
        {
            return -1;
        }
        char* value_data = (char*) malloc (sizeof (char) * (value_len + 1));
        if (NULL == value_data)
        {
            free (name_data);
            return -1;
        }

        memcpy (name_data, str, name_len);
        str += name_len;
        name_data[name_len] = '\0';
        memcpy (value_data, str, value_len);
        str += value_len;
        value_data[value_len] = '\0';

        fcgi_env_add (fcgi, name_data, value_data);
        printf ("    %s=%s \n", name_data, value_data);
    }

    ASSERT (accum == f->content_length);

    return 1;
};

static int
process_stdin (struct frame* f, struct fcgi* fcgi, const char* str, size_t num)
{
    char* _stdin;

    ASSERT (f);
    ASSERT (fcgi);
    ASSERT (str);

    printf ("process_stdin \n");

    _stdin = (char*) malloc (sizeof (byte) * (f->content_length + 1));
    if (NULL == _stdin)
    {
        printf ("ERROR \n");
        return 0;
    }

    memcpy (_stdin, str, f->content_length);
    _stdin[f->content_length] = '\0';
    fcgi->_stdin = _stdin;

    printf ("    stdin: %s \n", fcgi->_stdin);

    return 1;
};

static int
process_stdout (struct frame* f, struct fcgi* fcgi, const char* str, size_t num)
{
    return 1;
};

static int 
process_stderr (struct frame* f, struct fcgi* fcgi, const char* str, size_t num)
{
    return 1;
};

static int
process_data (struct frame* f, struct fcgi* fcgi, const char* str, size_t num)
{
    return 1;
};

static int 
process_get_values (struct frame* f, struct fcgi* fcgi, const char* str, size_t num)
{
    return 1;
};

static int 
process_get_values_result (struct frame* f, struct fcgi* fcgi, const char* str, size_t num)
{
    return 1;
};

static int
process_unknown_type (struct frame* f, struct fcgi* fcgi, const char* str, size_t num)
{
    return 1;
};


