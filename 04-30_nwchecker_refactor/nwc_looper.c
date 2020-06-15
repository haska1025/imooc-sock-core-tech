#include <stdlib.h>
#include "nwc_looper.h"

static nwc_handle_t nwc_looper_register_worker(struct nwc_looper *looper, void *ud, int (*worker_do)(void *ud))
{
    struct nwc_worker *worker = malloc(sizeof(struct nwc_worker));
    worker->worker_do = worker_do;
    worker->userdata = ud;

    list_add_tail(&worker->worker_entry, &looper->worker_hdr);

    return (nwc_handle_t)worker;
}
static int nwc_looper_cancel_worker(struct nwc_looper *looper, nwc_handle_t wk_handle)
{
    struct nwc_worker *wk = (struct nwc_worker*)wk_handle;
    list_del(&wk->worker_entry);

    wk->userdata = NULL;
    wk->worker_do = NULL;
    list_add_tail(&wk->worker_entry, &looper->removed_worker_hdr);

    return 0;
}

void nwc_looper_init(struct nwc_looper *looper)
{
    INIT_LIST_HEAD(&looper->worker_hdr);
    INIT_LIST_HEAD(&looper->removed_worker_hdr);

    looper->register_worker = nwc_looper_register_worker;
    looper->cancel_worker = nwc_looper_cancel_worker;
}

void nwc_looper_dispatch_worker(struct nwc_looper *looper)
{
    struct list_head *pos, *n;

    list_for_each_safe(pos, n, &looper->worker_hdr){
        struct nwc_worker *entry = list_entry(pos, struct nwc_worker, worker_entry);
        if (entry->worker_do){
            entry->worker_do(entry->userdata);
        }else{
            list_del(pos);
            free(entry);
            entry = NULL;
        }
    }
}
void nwc_looper_delete_removed_worker(struct nwc_looper *looper)
{
    struct list_head *pos, *n;

    list_for_each_safe(pos, n, &looper->removed_worker_hdr){
        struct nwc_worker *entry = list_entry(pos, struct nwc_worker, worker_entry);
        list_del(pos);
        free(entry);
        entry = NULL;
    }
}

