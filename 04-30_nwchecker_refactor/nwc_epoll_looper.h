#ifndef _EPOLL_LOOPER_H_
#define _EPOLL_LOOPER_H_

#include "list.h"
#include "nwc_types.h"

struct nwc_io_handler;

struct epoll_entry
{
    int mask;
    nwc_handle_t fd;
    struct nwc_io_handler *eh;
    struct list_head remove_entry;
};

struct nwc_epoll_looper
{
    int epfd;
    int exit;

    int timeout_interval;
    struct list_head removed_epoll_entrys;
};


int nwc_epoll_start(struct nwc_epoll_looper *looper);
int nwc_epoll_add_handler(struct nwc_epoll_looper *looper, struct nwc_io_handler *handler);
int nwc_epoll_remove_handler(struct nwc_epoll_looper *looper,struct nwc_epoll_looper * handle);
int nwc_epoll_register_event(struct nwc_epoll_looper *looper,struct nwc_epoll_looper * handle, int events);
int nwc_epoll_cancel_event(struct nwc_epoll_looper *looper,struct nwc_epoll_looper * handle, int events);
void nwc_epoll_run(struct nwc_epoll_looper *looper);
int nwc_epoll_stop(struct nwc_epoll_looper *looper);

#endif//_EPOLL_LOOPER_H_

