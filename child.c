#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "child.h"
#include "fcgi.h"
#include "debug.h"

typedef int content_func (int read_fd, int write_fd, struct child_cmd* cmd);

static content_func* cmd_handlers[3];

static int handle_app_exit (int read_fd, int write_fd, struct child_cmd* cmd);
static int handle_proc_exit (int read_fd, int write_fd, struct child_cmd* cmd);
static int handle_fcgi (int read_fd, int write_fd, struct child_cmd* cmd);

void 
init_child ()
{
    cmd_handlers[CMD_APP_EXIT] = handle_app_exit;
    cmd_handlers[CMD_PROC_EXIT] = handle_proc_exit;
    cmd_handlers[CMD_FCGI] = handle_fcgi;

    printf ("child initialized...\n");
};

int
child_run (pid_t pid, int read_fd, int write_fd)
{
    struct child_cmd cmd;

    ASSERT (pid > 0);
    ASSERT (read_fd > 2);
    ASSERT (write_fd > 2);

//    /* This function is run by the child processes.  They are
//     * designed to handle requests after fcgi has processed them
//     * and passed data in.  This function should not return until
//     * the child has gotten the notification from the parent that
//     * the application is quitting.
//     * */
//    do {
//        if (sizeof (int) != read (read_fd, (char*) &cmd, sizeof (struct child_cmd)))
//        {
//            printf ("Parent didn't send the right length of data \n");
//            break;
//        }
//
//        if (0 >= cmd_handlers[cmd.type])
//        {
//            printf ("Handler failed to return successfully. \n");
//            break;
//        }
//
//    } while (cmd->type != CMD_APP_EXIT && cmd->type != CMD_PROC_EXIT);

    printf ("Child here! %u \n", pid);
    return 0;
};

static int 
handle_app_exit (int read_fd, int write_fd, struct child_cmd* cmd){};

static int 
handle_proc_exit (int read_fd, int write_fd, struct child_cmd* cmd){};

static int 
handle_fcgi (int read_fd, int write_fd, struct child_cmd* cmd){};


