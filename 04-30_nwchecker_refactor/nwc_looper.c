#include "nwc_looper.h"


struct nwc_io_handler;
struct nwc_looper
{
    int (*start)(nwc_handle_t looper);
    int (*stop)(nwc_handle_t looper);
    int (*add_handler)(nwc_handle_t looper, struct nwc_io_handler *handler);
    int (*remove_handler)(nwc_handle_t looper,nwc_handle_t handle);
    int (*register_event)(nwc_handle_t looper,nwc_handle_t handle, int events);
    int (*cancel_event)(nwc_handle_t looper,nwc_handle_t handle, int events);
    void (*run)(nwc_handle_t looper);
};


nwc_handle_t nwc_looper_create()
{

}

void nwc_looper_destroy(nwc_handle_t looper)
{

}

