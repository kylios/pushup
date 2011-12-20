#ifndef __NETWORKING_H
#define __NETWORKING_H

int setnonblock (int fd);
int bind_addr_port (const char* addr, int ipver, const char* port);
int bind_unix_socket (const char* path);


#endif //__NETWORKING_H

