#ifndef _NWC_CONNECTION_H_
#define _NWC_CONNECTION_H_

#include <inttypes.h>
#include <netinet/in.h> 

#include "list.h"
#include "nwc.h"
#include "nwc_configuration.h"
#include "nwc_qos_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RECV_BUFFER_LEN 4096

struct nwc_connection
{
    struct nwc_io_operation op;
    struct list_head conn_entry;
    struct nwc_qos_protocol qos;
    nwc_handle_t handle;
    struct sockaddr_storage cliaddr;
    socklen_t cliaddrlen;
    uint8_t recv_buff[RECV_BUFFER_LEN];
    uint32_t rbytes;
    struct nwc_configuration *cfg;

    nwc_handle_t stat_wk_handle;
    nwc_handle_t send_wk_handle;
};

struct nwc_connection *alloc_nwc_conn_arg0();
void free_nwc_conn(struct nwc_connection *nwc);

void nwc_connection_udp_accept(nwc_handle_t handle, struct nwc_configuration *cfg);

void nwc_connection_init(struct nwc_connection *nwc, struct nwc_configuration *cfg);
void nwc_connection_on_connect(void *userdata);
int nwc_connection_on_recv(void *userdata);
int nwc_connection_on_send(void *userdata);
int nwc_connection_send(struct nwc_connection *nwc);
void nwc_connection_close(struct nwc_connection *nwc);

struct nwc_connection * get_nwc_connection(struct list_head *hdr, struct sockaddr *addr);
void add_nwc_tail(struct list_head *hdr, struct nwc_connection *nwc);
void remove_nwc(struct nwc_connection *nwc); 
void destroy_all_nwc(struct list_head *hdr);

#ifdef __cplusplus
}
#endif
#endif//_NWC_CONNECTION_H_
