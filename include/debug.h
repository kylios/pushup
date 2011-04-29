#ifndef DEBUG_H
#define DEBUG_H

#include "type.h"
#include <stdio.h>
#include <stdlib.h>

void debug_backtrace (void);
void hexdump (void* start, uint32 sz);

#define ASSERT(expression)  \
    if (!(expression))  {   \
        printf ("!!!Failed assertion: `%s' in %s: %d in %s \n\n", \
            #expression, __FILE__, __LINE__, __func__);    \
        debug_backtrace (); \
        exit(1); \
    } 

#define DEBUG_MARK  \
    printf ("DEBUG %s: %d (`%s')\n", __FILE__, __LINE__, __func__);

#define PANIC(MSG)  \
    printf ("!!!PANIC: %s \n", MSG); \
    exit(1);


#endif // DEBUG_H

