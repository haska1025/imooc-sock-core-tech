#include "nwc_looper.h"
#include "nwc_epoll_looper.h"
#include <stdlib.h>

nwc_handle_t nwc_looper_create()
{
    struct nwc_epoll_looper *looper = malloc(sizeof(struct nwc_epoll_looper));
    nwc_epoll_init(looper);

    return (nwc_handle_t)looper;
}

void nwc_looper_destroy(nwc_handle_t looper)
{
    struct nwc_looper *base_looper = (struct nwc_looper*)looper;
    base_looper->stop(looper); 
}

