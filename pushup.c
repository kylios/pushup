#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>

#include "pushup.h"
#include "debug.h"
#include "type.h"
#include "fcgi.h"

struct proc
{
    pid_t pid; 
    int status; // process's exit status
    bool exited;

    int write_fd; // socket to send data to the process
    int read_fd;  // socket to read data from the process

    pthread_mutex_t lock; 
    struct fcgi* fcgi; // fastcgi connection this process is handling
};

static struct proc procs[NUM_PROCS];
static unsigned int procs_available;
static pthread_mutex_t procs_lock;
static sem_t procs_count;

static int proc_init (struct proc*);

void
init_pushup ()
{
    unsigned int i;
    int ret;

    pthread_mutex_init (&procs_lock, NULL);
    sem_init (&procs_count, 0, NUM_PROCS);

    procs_available = NUM_PROCS;
    for (i = 0; i < NUM_PROCS; i++)
    {
        printf ("spawning child %u \n", i);
        struct proc* p = &procs[i];
        printf ("up \n");
        sem_wait (&procs_count);
        ret = proc_init (p);
        printf ("down \n");
        sem_post (&procs_count);
        if (-1 == ret)
        {
            printf ("Error in creation of child \n");
            return;
        }
        else if (0 != ret)
        {
            ASSERT (true == p->exited);
            exit (p->status);
        }
    }

    printf ("pushup initialized...\n");
};

void
end_pushup ()
{
    int i;

    for (i = 0; i < NUM_PROCS; i++)
    {
        printf ("waiting: %d \n", i);
        sem_wait (&procs_count);
    }
};

int
proc_init (struct proc* p)
{

    ASSERT (p);

    int writepipe[2] = {-1, -1};
    int readpipe[2] = {-1, -1};

    if (pipe (readpipe) < 0 || pipe (writepipe) < 0)
    {
        printf ("Failed to create pipes for child \n");
        return -1;
    }

    int childread = writepipe[0];
    int childwrite = readpipe[1];
    int parentread = readpipe[0];
    int parentwrite = writepipe[1];

    pid_t pid = fork ();
    if (pid == -1)
    {
        printf ("Failed to fork child \n");
        close (childread);
        close (childwrite);
        close (parentread);
        close (parentwrite);
        return -1;
    }

    if (pid != 0)
    {
        p->status = child_run (pid, childread, childwrite);
        p->exited = true;

        return 1;
    }
    else
    {
        p->exited = false;
        p->fcgi = NULL;
        p->pid = pid;
        p->write_fd = parentwrite;
        p->read_fd = parentread;
        pthread_mutex_init (&p->lock, NULL);

        return 0;
    }
};

