#include "nwc_looper.h"
#include "nwc_epoll_looper.h"
#include "nwc_tcp_listener.h"
#include <stdlib.h>

nwc_handle_t nwc_looper_create()
{
    struct nwc_epoll_looper *looper = malloc(sizeof(struct nwc_epoll_looper));
    nwc_epoll_init(looper);

    return (nwc_handle_t)looper;
}

void nwc_looper_destroy(nwc_handle_t looper)
{
    struct nwc_looper *blooper = (struct nwc_looper*)looper;
    free(blooper);
    blooper = NULL;
}

int nwc_looper_start(nwc_handle_t looper)
{
    struct nwc_looper *blooper = (struct nwc_looper*)looper;
    return blooper->start(blooper);
}
nwc_handle_t nwc_looper_add_handler(nwc_handle_t looper, struct nwc_io_handler *handler)
{
    struct nwc_looper *blooper = (struct nwc_looper*)looper;
    return blooper->add_handler(blooper, handler);
}
int nwc_looper_remove_handler(nwc_handle_t looper, nwc_handle_t handle)
{
    struct nwc_looper *blooper = (struct nwc_looper*)looper;
    return blooper->remove_handler(blooper, handle);
}
int nwc_looper_register_event(nwc_handle_t looper, nwc_handle_t handle, int events)
{
    struct nwc_looper *blooper = (struct nwc_looper*)looper;
    return blooper->register_event(blooper, handle, events);
}
int nwc_looper_cancel_event(nwc_handle_t looper, nwc_handle_t handle, int events)
{
    struct nwc_looper *blooper = (struct nwc_looper*)looper;
    return blooper->cancel_event(blooper, handle, events);
}
void nwc_looper_run(nwc_handle_t looper)
{
    struct nwc_looper *blooper = (struct nwc_looper*)looper;
    return blooper->run(blooper);
}
int nwc_looper_stop(nwc_handle_t looper)
{
    struct nwc_looper *blooper = (struct nwc_looper*)looper;
    return blooper->stop(blooper); 
}


nwc_handle_t nwc_tcp_listen_start(nwc_handle_t looper, const char *ip, uint16_t port)
{
    int rc = 0;
    struct nwc_tcp_listener *listener = malloc(sizeof(struct nwc_tcp_listener));
    listener->parent.looper = looper;
    rc = nwc_tcp_listener_open(listener, ip, port);
    if (rc != 0){
        free(listener);
        listener = NULL;
    }

    return (nwc_handle_t)listener;
}
int nwc_tcp_listen_stop(nwc_handle_t handle)
{
    struct nwc_tcp_listener *listener = (struct nwc_tcp_listener*)handle;
    nwc_tcp_listener_close(listener);

    return 0;
}

