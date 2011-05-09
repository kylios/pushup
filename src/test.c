#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions.h"
#include "user.h"
#include "type.h"
#include "debug.h"
#include "session.h"
#include "protocol.h"

int
main (int argc, char** argv)
{
    init_user_index ();
    init_session_index ();
    init_events ();

    // TEST 1
    register_user_session ("user1", "session1", NULL, NULL);
    user_debug ();

    // TEST 4
    push_user_session ("user1", "session1", "[message1]", NULL, NULL);
    user_debug ();

    // TEST 2
    register_user_session ("user2", "session1", NULL, NULL);
    register_user_session ("user1", "session2", NULL, NULL);
    user_debug ();

    // TEST 3
    register_user_session ("user2", "session2", NULL, NULL);
    user_debug ();

    // TEST 5
    push_user_session ("user1", "session1", "[message2,message3, message4 ]", NULL, NULL);
    user_debug ();

    push_user_session ("user1", "session2", "[ message5 ]", NULL, NULL);
    push_user_session ("user2", "session1", "[ message6, message7 ]", NULL, NULL);
    push_user_session ("user2", "session2", "[ message8, message9, message10 ]", NULL, NULL);

    user_debug ();

    printf ("\n\nNow begin fetching events\n");
    char* content = (char*) malloc (sizeof (char) * MESSAGE_STR_SZ);

    update_user_session ("user1", "session1", content, NULL, NULL);
    printf ("Message: %s\n", content);
    user_debug ();

    update_user_session ("user2", "session1", content, NULL, NULL);
    printf ("Message: %s\n", content);
    user_debug ();

    update_user_session ("user1", "session1", content, NULL, NULL);
    printf ("Message: %s\n", content);
    user_debug ();
};
