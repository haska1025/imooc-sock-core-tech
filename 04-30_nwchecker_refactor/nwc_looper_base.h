#ifndef _NWC_LOOPER_BASE_H_
#define _NWC_LOOPER_BASE_H_

#include "nwc_types.h"
#include "nwc_io_handler.h"

struct nwc_looper
{
    int (*start)(struct nwc_looper *looper);
    int (*stop)(void *looper);
    nwc_handle_t (*add_handler)(nwc_handle_t looper, struct nwc_io_handler *handler);
    int (*remove_handler)(nwc_handle_t looper,nwc_handle_t handle);
    int (*register_event)(nwc_handle_t looper,nwc_handle_t handle, int events);
    int (*cancel_event)(nwc_handle_t looper,nwc_handle_t handle, int events);
    void (*run)(nwc_handle_t looper);
};

#endif//_NWC_LOOPER_BASE_H_

