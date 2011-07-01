#ifndef FCGI_H
#define FCGI_H

#include <stddef.h>

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

struct fcgi_header
{
    int version;
    enum fcgi_type type;
    int request_id;
    size_t content_length;
    size_t padding_length;
};

struct fcgi_connection
{
    int role;
    char flags;
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

typedef int fcgi_record_processing_func (struct fcgi_header*, 
        struct fcgi_connection*, const char* buf);

int fcgi_read_header (int fd, struct fcgi_header* header);
void fcgi_dump_header (struct fcgi_header* header);

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

#endif //FCGI_H

