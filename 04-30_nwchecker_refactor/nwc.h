#ifndef _NWC_H_
#define _NWC_H_

#include "nwc_types.h"

struct nwc_io_handler;
struct nwc_worker;

nwc_handle_t nwc_looper_create();
void nwc_looper_destroy(nwc_handle_t looper);

int nwc_looper_start(nwc_handle_t looper);
nwc_handle_t nwc_looper_add_handler(nwc_handle_t looper, struct nwc_io_handler *handler);
int nwc_looper_remove_handler(nwc_handle_t looper, nwc_handle_t handle);
int nwc_looper_register_event(nwc_handle_t looper, nwc_handle_t handle, int events);
int nwc_looper_cancel_event(nwc_handle_t looper, nwc_handle_t handle, int events);
void nwc_looper_run(nwc_handle_t looper);
int nwc_looper_stop(nwc_handle_t looper);
nwc_handle_t nwc_looper_register_worker(nwc_handle_t looper, void *ud, int (*worker_do)(void *ud));
int nwc_looper_cancel_worker(nwc_handle_t looper, nwc_handle_t wk_handle);

struct nwc_io_operation
{
    void *userdata;

    struct nwc_io_operation *(*on_accept)(void *userdata, nwc_handle_t handle);
    void (*on_connect)(void *userdata);
    int (*on_recv)(void *userdata);
    int (*on_send)(void *userdata);
};

void nwc_io_operation_init(struct nwc_io_operation *op);

nwc_handle_t nwc_io_create(nwc_handle_t looper, struct nwc_io_operation *op, int prototype);

int nwc_io_listen(nwc_handle_t looper, const char *ip, uint16_t port);
int nwc_io_connect(nwc_handle_t looper, const char *ip, uint16_t port);
int nwc_io_send(nwc_handle_t handle, uint8_t buff[], uint32_t bufflen);
int nwc_io_sendto(nwc_handle_t handle, uint8_t buff[], uint32_t bufflen, struct sockaddr *toaddr, socklen_t toaddrlen);
int nwc_io_recv(nwc_handle_t handle, uint8_t buff[], uint32_t bufflen);
int nwc_io_recvfrom(nwc_handle_t handle, uint8_t buff[], uint32_t bufflen, struct sockaddr *fromaddr, socklen_t *fromaddrlen);
int nwc_io_can_send(nwc_handle_t handle);
int nwc_io_close(nwc_handle_t handle);
nwc_handle_t nwc_io_get_looper(nwc_handle_t handle);


#endif//_NWC_H_

