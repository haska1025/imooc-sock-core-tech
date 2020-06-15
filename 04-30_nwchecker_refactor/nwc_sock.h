#ifndef _NWC_SOCK_H_
#define _NWC_SOCK_H_

#include <inttypes.h>

int nwc_sock_create(int family, int socktype, int protocol, int nonblock, int reuseaddr);
int nwc_sock_listen(int family, int socktype, const char *host, uint16_t port);
int nwc_sock_connect(int family, int socktype, const char *peerhost, uint8_t peerport, int *fd);

#endif//_NWC_SOCK_H_

