#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "protocol.h"
#include "lib/mongoose.h"
#include "type.h"


/**
 * Return the value of an http variable by evaluating the query string and
 * post data. 
 * */
static int
get_var (const struct mg_request_info* info, const char* request_body,
        const char* var, char* dst, size_t dst_len)
{
    const char* qs = (NULL == info->query_string ? "" : info->query_string);
    size_t sz = strlen (qs);

    int result = mg_get_var (qs, sz, var, dst, dst_len);
    if (result == -1)
    {
        /*
         * mg_get_var returns -1 if there was an error or if the variable was
         * not found in the string.  If the variable was not found, we don't
         * want to return -1, we want to try the next string.  If the variable
         * was found but -1 was returned, then an error did occur and we need to
         * return -1.
         * */
        if (NULL == strstr (qs, var))
        {
            const char* rb = request_body;
            sz = strlen (rb == NULL ? "" : rb);

            result = mg_get_var (rb, sz, var, dst, dst_len);
            if (result == -1)
            {
                /*
                 * Once again, check if the variable is in the string.  If it is
                 * not, then we return 0 instead of -1.
                 * */
                if (NULL == strstr (rb, var))
                {
                    result = 0;
                }
            }
        }
    }

    return result;
};

reqtype_t 
protocol_eval (protocol_info_t* pinfo, const char* request_body,
        const struct mg_request_info* rinfo)
{
    /*
     * Get the variables
     * */
    size_t action_buf_sz = 1 + ACTION_STR_SZ;
    char* action_buf = 
        (char*) malloc (sizeof (char) * action_buf_sz);
    size_t user_buf_sz = 1 + USER_STR_SZ;
    char* user_buf = 
        (char*) malloc (sizeof (char) * user_buf_sz);
    size_t session_buf_sz = 1 + SESSION_STR_SZ;
    char* session_buf = 
        (char*) malloc (sizeof (char) * session_buf_sz);
    size_t message_buf_sz = 1 + MESSAGE_STR_SZ;
    char* message_buf = 
        (char*) malloc (sizeof (char) * message_buf_sz);

    if (NULL == action_buf || NULL == user_buf || 
        NULL == session_buf || NULL == message_buf)
    {
        free (action_buf);
        free (user_buf);
        free (session_buf);
        free (message_buf);

        return PR_DEFAULT;
    }

    printf ("Request Body: %s\n", request_body);

    int result = 1;
    result |= get_var (rinfo, request_body, PROTO_ACTION, 
            action_buf, sizeof (char) * action_buf_sz);            
    result |= get_var (rinfo, request_body, PROTO_USER, 
            user_buf, sizeof (char) * user_buf_sz);
    result |= get_var (rinfo, request_body, PROTO_SESSION, 
            session_buf, sizeof (char) * session_buf_sz);
    result |= get_var (rinfo, request_body, PROTO_MESSAGE, 
            message_buf, sizeof (char) * message_buf_sz);

    /*
     * Error occurred if any of the above return -1.
     * */
    if (result < 0)
    {
        free (action_buf);
        free (user_buf);
        free (session_buf);
        free (message_buf);

        return PR_DEFAULT;
    }

    /*
     * Do some checks to make sure the right arguments are supplied.
     * */
    pinfo->a = action_buf;
    pinfo->u = user_buf;
    pinfo->s = session_buf;
    pinfo->m = message_buf;

    /*
     * Determine the request type using the action (a) field, and populate the
     * reqtype field.  Also perform checks to ensure all the right variables are
     * supplied depending on the request type.  See README for detailed protocol
     * information.
     * */
    switch (pinfo->a[0])
    {
        case 'p':
            pinfo->reqtype = PR_PUSH;
            require (pinfo->u);
            require (pinfo->s);
            require (pinfo->m);
            break;
        case 'u':
            pinfo->reqtype = PR_UPDATE;
            require (pinfo->u);
            require (pinfo->s);
            break;
        case 'r':
            pinfo->reqtype = PR_REG;
            require (pinfo->u);
            require (pinfo->s);
            break;
        case 'd':
            pinfo->reqtype = PR_UREG;
            require (pinfo->u);
            require (pinfo->s);
            break;
        default:
            pinfo->reqtype = PR_DEFAULT;
            break;
    }

    return pinfo->reqtype;
};
