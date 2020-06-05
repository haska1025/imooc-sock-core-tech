#ifndef _NWC_CONNECTION_H_
#define _NWC_CONNECTION_H_

#include "list.h"
#include <inttypes.h>
#include <netinet/in.h> 

#ifdef __cplusplus
extern "C" {
#endif

#define RECV_BUFFER_LEN 4096

struct nwc_connection
{
    struct list_head conn_entry;
    int fd;
    struct sockaddr_storage cliaddr;
    uint8_t recv_buff[RECV_BUFFER_LEN];
    uint32_t rbytes;
};

struct nwc_connection *alloc_nwc_conn_arg0();
void free_nwc_conn(struct nwc_connection *nwc);

struct nwc_connection * get_nwc_connection(struct list_head *hdr, struct sockaddr *addr);

void add_nwc_tail(struct list_head *hdr, struct nwc_connection *nwc);
void remove_nwc(struct nwc_connection *nwc); 
void destroy_all_nwc(struct list_head *hdr);

#ifdef __cplusplus
}
#endif
#endif//_NWC_CONNECTION_H_
