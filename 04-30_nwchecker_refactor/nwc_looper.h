#ifndef _NWC_LOOPER_BASE_H_
#define _NWC_LOOPER_BASE_H_

#include "nwc_types.h"
#include "list.h"

struct nwc_io_handler;

struct nwc_looper
{
    int (*start)(struct nwc_looper *looper);
    int (*stop)(struct nwc_looper *looper);
    nwc_handle_t (*add_handler)(struct nwc_looper *looper, struct nwc_io_handler *handler);
    int (*remove_handler)(struct nwc_looper *looper,nwc_handle_t handle);
    int (*register_event)(struct nwc_looper *looper,nwc_handle_t handle, int events);
    int (*cancel_event)(struct nwc_looper *looper,nwc_handle_t handle, int events);
    void (*run)(struct nwc_looper *looper);
    nwc_handle_t (*register_worker)(struct nwc_looper *looper, void *ud, int (*worker_do)(void *ud));
    int (*cancel_worker)(struct nwc_looper *looper, nwc_handle_t wk_handle);

    struct list_head worker_hdr;
    struct list_head removed_worker_hdr;
};

void nwc_looper_init(struct nwc_looper *looper);

struct nwc_worker
{
    void *userdata;
    int (*worker_do)(void *userdata);
    struct list_head worker_entry;
};

void nwc_looper_dispatch_worker(struct nwc_looper *looper);
void nwc_looper_delete_removed_worker(struct nwc_looper *looper);


#endif//_NWC_LOOPER_BASE_H_

