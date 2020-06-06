#include <sys/socket.h>

#include "nwc_tcp_listener.h"
#include "nwc.h"
#include "nwc_sock.h"

int nwc_tcp_listener_open(struct nwc_tcp_listener *listener, const char *ip, uint16_t port)
{
    int sock_fd = -1;

    sock_fd = nwc_sock_listen(AF_INET, SOCK_STREAM, ip, port);
    if (sock_fd == -1)
        return -1;

    nwc_io_handler_init(&listener->parent);
    listener->parent.handle_input = nwc_tcp_listener_handle_input;
    listener->parent.fd = sock_fd;

    listener->parent.h_handle = nwc_looper_add_handler(listener->parent.looper, (struct nwc_io_handler*)&listener);

    return 0;
}
void nwc_tcp_listener_handle_input(struct nwc_io_handler *handler)
{
}

int nwc_tcp_listener_close(struct nwc_tcp_listener *listener)
{
    return 0;
}

