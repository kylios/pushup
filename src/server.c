#include "../fcgi-2.4.0/include/fcgi_stdio.h"
#include <stdlib.h>

void
main (int argc, char** argv)
{
    int count = 0;
    while (FCGI_Accept () >= 0)
    {
        printf ("Content-Type: text/html\r\n"
                "\r\n"
                "<title>FastCGI Hello!</title>\r\n"
                "<h1>Hello World!</h1>\r\n"
                "Request number %d running on host %s\n",
                count++, getenv ("SERVER_NAME"));
    }
};
