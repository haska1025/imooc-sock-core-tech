#ifndef _EPOLL_LOOPER_H_
#define _EPOLL_LOOPER_H_

#include "list.h"
#include "nwc_types.h"
#include "nwc_looper.h"

struct nwc_io_handler;

struct epoll_entry
{
    int mask;
    int fd;
    struct nwc_io_handler *eh;
    struct list_head remove_entry;
};

struct nwc_epoll_looper
{
    struct nwc_looper parent;
    int epfd;
    int exit;

    int timeout_interval;
    struct list_head removed_epoll_entrys;
};

int nwc_epoll_init(struct nwc_epoll_looper *looper);
int nwc_epoll_start(struct nwc_looper *looper);
nwc_handle_t nwc_epoll_add_handler(struct nwc_looper *looper, struct nwc_io_handler *handler);
int nwc_epoll_remove_handler(struct nwc_looper *looper, nwc_handle_t handle);
int nwc_epoll_register_event(struct nwc_looper *looper, nwc_handle_t handle, int events);
int nwc_epoll_cancel_event(struct nwc_looper *looper, nwc_handle_t handle, int events);
void nwc_epoll_run(struct nwc_looper *looper);
int nwc_epoll_stop(struct nwc_looper *looper);


#define epoll_looper(looper)\
    (struct nwc_epoll_looper *)(looper)

#endif//_EPOLL_LOOPER_H_

