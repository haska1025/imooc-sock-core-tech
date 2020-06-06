#include <errno.h>

#include "nwc_tcp_handler.h"
#include "nwc_sock.h"
#include "nwc.h"

int nwc_tcp_handler_init(struct nwc_tcp_handler *handler)
{
    nwc_io_handler_init(&handler->parent);
    handler->parent.handle_input = nwc_tcp_handle_input;
    handler->parent.handle_output = nwc_tcp_handle_output;
    handler->state = NTS_CLOSED;
}
int nwc_tcp_handler_connect(struct nwc_tcp_handler *handler, const char *hostname, uint16_t port)
{
    int fd = -1;
    int rc = 0;
    int events = EM_READ;

    rc= nwc_sock_connect(hostname, port, &fd);
    if (rc == 0){
        handler->state = NTS_ESTABLISHED;
    }else if(rc == EINPROGRESS || rc == EWOULDBLOCK){
        events |= EM_WRITE;
        handler->state = NTS_CONNECTING;
    }else{
        // error
        return rc;
    }

    handler->parent.fd = fd;
    handler->parent.h_handle = nwc_looper_add_handler(handler->parent.looper, &handler->parent); 
    nwc_looper_register_event(handler->parent.looper, handler->parent.h_handle, events);

    return 0;
}
int nwc_tcp_handler_accept(struct nwc_tcp_handler *handler, int fd)
{
    handler->parent.fd = fd;
    handler->parent.h_handle = nwc_looper_add_handler(handler->parent.looper, &handler->parent); 
    nwc_looper_register_event(handler->parent.looper, handler->parent.h_handle, EM_READ);

    return 0;
}
int nwc_tcp_handler_send(struct nwc_tcp_handler *handler, const uint8_t buffer[], uint32_t buflen)
{
}
int nwc_tcp_handler_close(struct nwc_tcp_handler *handler)
{
    handler->state = NTS_CLOSED;

}

void nwc_tcp_handle_input(struct nwc_io_handler *handler)
{
}
void nwc_tcp_handle_output(struct nwc_io_handler *handler)
{
}

