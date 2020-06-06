#ifndef _NWC_TCP_LISTENER_
#define _NWC_TCP_LISTENER_

#include "nwc_io_handler.h"

struct nwc_tcp_listener
{
    struct nwc_io_handler parent;
};

int nwc_tcp_listener_open(struct nwc_tcp_listener *listener, const char *ip, uint16_t port);
int nwc_tcp_listener_close(struct nwc_tcp_listener *listener);
void nwc_tcp_listener_handle_input(struct nwc_io_handler *handler);

#endif//_NWC_TCP_LISTENER_

