#ifndef _NWC_IO_HANDLER_H_
#define _NWC_IO_HANDLER_H_

#include "nwc_types.h"
#include "nwc.h"

#define EM_READ 1
#define EM_WRITE 2
#define EM_ALL (EM_READ | EM_WRITE)

// The base io handler
struct nwc_io_handler
{
    void (*handle_input)(struct nwc_io_handler *handler);
    void (*handle_output)(struct nwc_io_handler *handler);

    int (*listen)(struct nwc_io_handler *handler, const char *ip, uint16_t port);
    int (*connect)(struct nwc_io_handler *handler, const char *hostname, uint16_t port);
    int (*accept)(struct nwc_io_handler *handler, int fd);
    int (*send)(struct nwc_io_handler *handler, const uint8_t buffer[], uint32_t buflen);
    int (*recv)(struct nwc_io_handler *handler, uint8_t buffer[], uint32_t buflen);
    int (*sendto)(struct nwc_io_handler *handler, const uint8_t buffer[], uint32_t buflen, struct sockaddr *toaddr, socklen_t toaddrlen);
    int (*recvfrom)(struct nwc_io_handler *handler, uint8_t buffer[], uint32_t buflen, struct sockaddr *fromaddr, socklen_t *fromaddrlen);
    int (*close)(struct nwc_io_handler *handler);

    nwc_handle_t looper;
    int fd; // socket file description
    nwc_handle_t h_handle; // The handle for io_handler
    struct nwc_io_operation *ioop;
    int can_send;
};

static inline void nwc_io_handler_init(struct nwc_io_handler *h)
{
    h->handle_input = NULL;
    h->handle_output = NULL;

    h->listen = NULL;
    h->connect = NULL;
    h->send = NULL;
    h->sendto = NULL;
    h->recv = NULL;
    h->recvfrom = NULL; 
    h->close = NULL;

    h->looper = INVALID_HANDLE;
    h->fd = -1;
    h->h_handle = INVALID_HANDLE;
    h->ioop = NULL;
    h->can_send = 1;
}

#endif//_NWC_IO_HANDLER_H_

