#include <stdlib.h>

#include "nwc_looper.h"
#include "nwc_epoll_looper.h"
#include "nwc_tcp_handler.h"
#include "nwc_udp_handler.h"
#include "nwc.h"

nwc_handle_t nwc_looper_create()
{
    struct nwc_epoll_looper *looper = malloc(sizeof(struct nwc_epoll_looper));
    nwc_epoll_init(looper);
    looper->timeout_interval = SCHEDULE_TIMER;

    return (nwc_handle_t)looper;
}

void nwc_looper_destroy(nwc_handle_t looper)
{
    if (looper == INVALID_HANDLE)
        return;

    struct nwc_looper *blooper = (struct nwc_looper*)looper;
    free(blooper);
    blooper = NULL;
}

int nwc_looper_start(nwc_handle_t looper)
{
    if (looper == INVALID_HANDLE)
        return -1;

    struct nwc_looper *blooper = (struct nwc_looper*)looper;
    return blooper->start(blooper);
}
nwc_handle_t nwc_looper_add_handler(nwc_handle_t looper, struct nwc_io_handler *handler)
{
    if (looper == INVALID_HANDLE || handler == NULL)
        return INVALID_HANDLE;

    struct nwc_looper *blooper = (struct nwc_looper*)looper;
    return blooper->add_handler(blooper, handler);
}
int nwc_looper_remove_handler(nwc_handle_t looper, nwc_handle_t handle)
{
    if (looper == INVALID_HANDLE || handle == INVALID_HANDLE)
        return -1;

    struct nwc_looper *blooper = (struct nwc_looper*)looper;
    return blooper->remove_handler(blooper, handle);
}
int nwc_looper_register_event(nwc_handle_t looper, nwc_handle_t handle, int events)
{
    if (looper == INVALID_HANDLE || handle == INVALID_HANDLE)
        return -1;

    struct nwc_looper *blooper = (struct nwc_looper*)looper;
    return blooper->register_event(blooper, handle, events);
}
int nwc_looper_cancel_event(nwc_handle_t looper, nwc_handle_t handle, int events)
{
    if (looper == INVALID_HANDLE || handle == INVALID_HANDLE)
        return -1;

    struct nwc_looper *blooper = (struct nwc_looper*)looper;
    return blooper->cancel_event(blooper, handle, events);
}
void nwc_looper_run(nwc_handle_t looper)
{
    if (looper == INVALID_HANDLE)
        return ;
    
    struct nwc_looper *blooper = (struct nwc_looper*)looper;
    return blooper->run(blooper);
}
int nwc_looper_stop(nwc_handle_t looper)
{
    if (looper == INVALID_HANDLE)
        return -1;
    
    struct nwc_looper *blooper = (struct nwc_looper*)looper;
    return blooper->stop(blooper); 
}

nwc_handle_t nwc_looper_register_worker(nwc_handle_t looper, void *ud, int (*worker_do)(void *ud))
{
    if (looper == INVALID_HANDLE || worker_do == NULL)
        return INVALID_HANDLE;
    
    struct nwc_looper *blooper = (struct nwc_looper*)looper;
    return blooper->register_worker(blooper, ud, worker_do);
}
int nwc_looper_cancel_worker(nwc_handle_t looper, nwc_handle_t wk_handle)
{
    if (looper == INVALID_HANDLE || wk_handle == INVALID_HANDLE)
        return -1;
    
    struct nwc_looper *blooper = (struct nwc_looper*)looper;
    return blooper->cancel_worker(blooper, wk_handle);
}

nwc_handle_t nwc_io_create(nwc_handle_t looper, struct nwc_io_operation *op, int prototype)
{
    struct nwc_io_handler *io = NULL;

    if (looper == INVALID_HANDLE || op == NULL)
        return INVALID_HANDLE;

    if (prototype == NWC_PROTO_TCP){
        io = malloc(sizeof(struct nwc_tcp_handler));
        nwc_tcp_handler_init((struct nwc_tcp_handler*)io);
    }else{
        io = malloc(sizeof(struct nwc_udp_handler));
        nwc_udp_handler_init((struct nwc_udp_handler*)io);
    }

    io->ioop = op;
    io->looper = looper;

    return (nwc_handle_t)io;
}

int nwc_io_listen(nwc_handle_t handle, const char *ip, uint16_t port)
{
    struct nwc_io_handler *io = (struct nwc_io_handler*)handle;

    return io->listen(io, ip, port);
}
int nwc_io_connect(nwc_handle_t handle, const char *ip, uint16_t port)
{
    struct nwc_io_handler *io = (struct nwc_io_handler*)handle;

    return io->connect(io, ip,  port);
}
int nwc_io_send(nwc_handle_t handle, uint8_t buff[], uint32_t bufflen)
{
    struct nwc_io_handler *io = (struct nwc_io_handler*)handle;

    return io->send(io, buff, bufflen);
}
int nwc_io_recv(nwc_handle_t handle, uint8_t buff[], uint32_t bufflen)
{
    struct nwc_io_handler *io = (struct nwc_io_handler*)handle;
    return io->recv(io, buff, bufflen);
}

int nwc_io_sendto(nwc_handle_t handle, uint8_t buff[], uint32_t bufflen, struct sockaddr *toaddr, socklen_t toaddrlen)
{
    struct nwc_io_handler *io = (struct nwc_io_handler*)handle;

    return io->sendto(io, buff, bufflen, toaddr, toaddrlen);
}
int nwc_io_recvfrom(nwc_handle_t handle, uint8_t buff[], uint32_t bufflen, struct sockaddr *fromaddr, socklen_t *fromaddrlen)
{
    struct nwc_io_handler *io = (struct nwc_io_handler*)handle;
    return io->recvfrom(io, buff, bufflen, fromaddr, fromaddrlen);
}

int nwc_io_close(nwc_handle_t handle)
{
    struct nwc_io_handler *io = (struct nwc_io_handler*)handle;
    
    io->close(io);

    free(io);
    io = NULL;

    return 0;
}

int nwc_io_can_send(nwc_handle_t handle)
{
    struct nwc_io_handler *io = (struct nwc_io_handler*)handle;
    return io->can_send;
}
nwc_handle_t nwc_io_get_looper(nwc_handle_t handle)
{
    struct nwc_io_handler *io = (struct nwc_io_handler*)handle;
    return io->looper;
}

void nwc_io_operation_init(struct nwc_io_operation *op)
{
    op->userdata = NULL;
    op->on_accept = NULL;
    op->on_connect = NULL;
    op->on_recv = NULL;
    op->on_send = NULL;
}

