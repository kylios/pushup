#ifndef __DEBUG_H
#define __DEBUG_H

#include <stdlib.h>
#include <stdio.h>

void debug_backtrace (void);
void hexdump (void* start, size_t sz);

#define ASSERT(expression)  \
    if (!(expression))  {   \
        printf ("!!!Failed assertion: `%s' in %s: %d in %s \n\n", \
            #expression, __FILE__, __LINE__, __func__);    \
        debug_backtrace (); \
        while (1); \
    }

#define DEBUG_MARK  \
    printf ("DEBUG %s: %d (`%s')\n", __FILE__, __LINE__, __func__);

#endif //__DEBUG_H

