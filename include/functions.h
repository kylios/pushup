#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "user.h"
#include "session.h"
#include "event_queue.h"

bool register_user_session (const char* uid, const char* sid, user_t**, session_t**);
void unregister_user_session (const char* uid, const char* sid);
bool push_user_session (const char* uid, const char* sid, const char* message,
        user_t** u, session_t** s);
bool update_user_session (const char* uid, const char* sid, char* message, 
        user_t** u, session_t** s);
bool user_update_func_cb (user_t* user, session_t* session, event_queue_t* eq);

#endif //FUNCTIONS_H
