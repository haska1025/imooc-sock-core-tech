#ifndef _NWC_LOOPER_BASE_H_
#define _NWC_LOOPER_BASE_H_

#include "nwc_types.h"
#include "nwc_io_handler.h"

struct nwc_looper
{
    int (*start)(struct nwc_looper *looper);
    int (*stop)(struct nwc_looper *looper);
    nwc_handle_t (*add_handler)(struct nwc_looper *looper, struct nwc_io_handler *handler);
    int (*remove_handler)(struct nwc_looper *looper,nwc_handle_t handle);
    int (*register_event)(struct nwc_looper *looper,nwc_handle_t handle, int events);
    int (*cancel_event)(struct nwc_looper *looper,nwc_handle_t handle, int events);
    void (*run)(struct nwc_looper *looper);
};

#endif//_NWC_LOOPER_BASE_H_

