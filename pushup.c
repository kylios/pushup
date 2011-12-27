#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <semaphore.h>

#include "pushup.h"
#include "child.h"
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

// List of worker processes
static struct proc procs[NUM_PROCS];

// Number of workers available to process fcgi requests
static unsigned int procs_available;
// Protects procs_available 
static pthread_mutex_t procs_lock;

// List of fcgi requests waiting to be processed
static struct list wait_queue;
static pthread_mutex_t wait_queue_lock;


static int proc_init (struct proc*);

static struct proc* get_available_proc ();
static void assign_proc (struct proc* p, struct fcgi* fcgi);

void
init_pushup ()
{
    unsigned int i;
    int ret;

    pthread_mutex_init (&procs_lock, NULL);
    pthread_mutex_init (&wait_queue_lock, NULL);

    list_init (&wait_queue);

    procs_available = NUM_PROCS;
    for (i = 0; i < NUM_PROCS; i++)
    {
        printf ("spawning child %u \n", i);
        struct proc* p = &procs[i];
        ret = proc_init (p);
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
    struct proc* p;

    for (i = 0; i < NUM_PROCS; i++)
    {
        p = &procs[i];

        printf ("waiting: %d \n", p->pid);
        waitpid (p->pid, &p->status, 0);
    }
    
    /* All children are dead */
};

int
proc_init (struct proc* p)
{
    ASSERT (p);

    dup (0);
    dup (1);
    dup (2);

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

int 
pushup_dispatch (struct fcgi* fcgi)
{
    struct proc* p;

    ASSERT (fcgi);

    /*
     * Check how many workers are available to process requests.  
     * If there are none available, save the requests in a queue to be
     * assigned later. */
    pthread_mutex_lock (&procs_lock);
    if (0 == procs_available)
    {
        pthread_mutex_unlock (&procs_lock);

        pthread_mutex_lock (&wait_queue_lock);
        list_push_back (&wait_queue, &fcgi->elem);
        pthread_mutex_unlock (&wait_queue_lock);
    }
    else
    {
        p = get_available_proc ();
        ASSERT (NULL != p);

        assign_proc (p, fcgi);

        pthread_mutex_unlock (&p->lock);
        pthread_mutex_unlock (&procs_lock);
    }
};

static struct proc* 
get_available_proc ()
{
    int i;

    ASSERT (procs_available > 0);
    // ASSERT (we hold procs_lock)

    for (i = 0; i < NUM_PROCS; i++)
    {
        pthread_mutex_lock (&procs[i].lock);
        if (procs[i].fcgi == NULL)
        {
            return &procs[i];
        }
        pthread_mutex_unlock (&procs[i].lock);
    }

    // Our ASSERTs should prevent us from logically getting here
    NOT_REACHED;
};

static void
assign_proc (struct proc* p, struct fcgi* fcgi)
{
    struct child_cmd cmd;

    ASSERT (p);
    ASSERT (p->fcgi == NULL);
    ASSERT (fcgi);
    // ASSERT (we hold p->lock)

    p->fcgi = fcgi;

    cmd.type = CMD_FCGI;
    cmd.content_length = sizeof (struct fcgi);
    write (p->write_fd, &cmd, sizeof (struct child_cmd));
    write (p->write_fd, p->fcgi, sizeof (struct fcgi));
};

