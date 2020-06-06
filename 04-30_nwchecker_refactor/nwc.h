#ifndef _NWC_H_
#define _NWC_H_

#include "nwc_types.h"

struct nwc_io_handler;

nwc_handle_t nwc_looper_create();
void nwc_looper_destroy(nwc_handle_t looper);

int nwc_looper_start(nwc_handle_t looper);
nwc_handle_t nwc_looper_add_handler(nwc_handle_t looper, struct nwc_io_handler *handler);
int nwc_looper_remove_handler(nwc_handle_t looper, nwc_handle_t handle);
int nwc_looper_register_event(nwc_handle_t looper, nwc_handle_t handle, int events);
int nwc_looper_cancel_event(nwc_handle_t looper, nwc_handle_t handle, int events);
void nwc_looper_run(nwc_handle_t looper);
int nwc_looper_stop(nwc_handle_t looper);

nwc_handle_t nwc_tcp_listen_start(nwc_handle_t looper, const char *ip, uint16_t port);
int nwc_tcp_listen_stop(nwc_handle_t handle);

#endif//_NWC_H_

