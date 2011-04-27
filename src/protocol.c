#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "protocol.h"
#include "lib/mongoose.h"
#include "type.h"


/**
 * Return the value of an http variable by evaluating the query string and
 * postdate. 
 * */
static int
get_var (const struct mg_request_info* info, char* request_body,
        const char* buf, size_t buf_len, 
        const char* var, char* dst, size_t dst_len)
{
    const char* qs = info->query_string;
    size_t sz = strlen (qs == NULL ? "" : qs);

    int result = mg_get_var (qs, sz, var, dst, dst_len);
//    if (result == -1)
//    {
//        const char* postdata = info->
//    }
};

reqtype_t 
protocol_eval (protocol_info_t* pinfo, const char* request_body,
        const struct mg_request_info* rinfo)
{
    /*
     * Get the variables
     * */
    char* action_buffer = 
        (char*) malloc (sizeof (char) * (1 + ACTION_STR_SZ));
    char* user_buffer = 
        (char*) malloc (sizeof (char) * (1 + USER_STR_SZ));
    char* session_buffer = 
        (char*) malloc (sizeof (char) * (1 + SESSION_STR_SZ));
    char* message_buffer = 
        (char*) malloc (sizeof (char) * (1 + MESSAGE_STR_SZ));

    if (NULL == action_buffer || NULL == user_buffer || 
        NULL == session_buffer || NULL == message_buffer)
    {
        free (action_buffer);
        free (user_buffer);
        free (session_buffer);
        free (message_buffer);

        return PR_DEFAULT;
    }
    int result = 1;
    const char* qs = rinfo->query_string;
    size_t sz = strlen(qs == NULL ? "" : qs);
    result |= 
        mg_get_var (qs, sz, 
        PROTO_ACTION, action_buffer, sizeof (char) * (1 + ACTION_STR_SZ));            
    result |= 
        mg_get_var (qs, sz,
        PROTO_USER, user_buffer, sizeof (char) * (1 + USER_STR_SZ));
    result |= 
        mg_get_var (qs, sz,
        PROTO_SESSION, session_buffer, sizeof (char) * (1 + SESSION_STR_SZ));
    result |= 
        mg_get_var (qs, sz,
        PROTO_MESSAGE, message_buffer, sizeof (char) * (1 + MESSAGE_STR_SZ));

    if (result <= 0)
    {
        free (action_buffer);
        free (user_buffer);
        free (session_buffer);
        free (message_buffer);

        return PR_DEFAULT;
    }

    pinfo->a = action_buffer;
    pinfo->u = user_buffer;
    pinfo->s = session_buffer;
    pinfo->m = message_buffer;

    /*
     * Determine the request type using the action (a) field, and populate the
     * reqtype field
     * */
    switch (pinfo->a[0])
    {
        case 'p':
            pinfo->reqtype = PR_PUSH;
            break;
        case 'u':
            pinfo->reqtype = PR_UPDATE;
            break;
        case 'r':
            pinfo->reqtype = PR_REG;
            break;
        case 'd':
            pinfo->reqtype = PR_UREG;
            break;
        default:
            pinfo->reqtype = PR_DEFAULT;
            break;
    }

    return pinfo->reqtype;
};
