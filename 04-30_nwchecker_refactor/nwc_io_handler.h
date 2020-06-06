#ifndef _NWC_IO_HANDLER_H_
#define _NWC_IO_HANDLER_H_

#include "nwc_types.h"

#define EM_READ 1
#define EM_WRITE 2

struct nwc_io_handler
{
    void (*handle_input)(struct nwc_io_handler *handler);
    void (*handle_output)(struct nwc_io_handler *handler);

    nwc_handle_t looper;
    int fd;
    nwc_handle_t h_handle; // The handle for io_handler
};

static inline void nwc_io_handler_init(struct nwc_io_handler *h)
{
    h->handle_input = 0;
    h->handle_output = 0;
    h->looper = INVALID_HANDLE;
    h->fd = -1;
    h->h_handle = INVALID_HANDLE;
}

#endif//_NWC_IO_HANDLER_H_

