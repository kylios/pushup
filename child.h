#ifndef __CHILD_H
#define __CHILD_H

#include <sys/types.h>
#include <unistd.h>


enum child_cmd_type 
{
    CMD_APP_EXIT = 0,
    CMD_PROC_EXIT = 1,
    CMD_FCGI = 2
};

void init_child ();

/* Defines a command sent to a child process.
 * See enum child_cmd_type for types of commands 
 * that can be sent.
 * */
struct child_cmd
{
    enum child_cmd_type type;
    size_t content_length;
};

int child_run (pid_t pid, int childread, int childwrite);

#endif //__CHILD_H

