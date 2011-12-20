#ifndef __PUSHUP_H
#define __PUSHUP_H

#include "fcgi.h"

/* Number of processes to spawn.  
 * TODO: move to configuration file 
 * */
#define NUM_PROCS 5

void init_pushup ();
void end_pushup ();
int pushup_dispatch (struct fcgi*);


#endif //__PUSHUP_H

