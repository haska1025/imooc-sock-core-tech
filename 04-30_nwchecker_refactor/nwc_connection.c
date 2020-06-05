#include <stdlib.h>
#include <unistd.h>

#include "nwc_connection.h"

/////////////////// The nwc connection ////////////////////////////////
struct nwc_connection *alloc_nwc_conn_arg0()
{
    struct nwc_connection *nwc = (struct nwc_connection*)malloc(sizeof(struct nwc_connection));

    INIT_LIST_HEAD(&nwc->conn_entry);
    nwc->rbytes = 0;

    return nwc;
}


void free_nwc_conn(struct nwc_connection *nwc)
{
    if (nwc){
        if (nwc->fd != -1) {
            close(nwc->fd);
            nwc->fd = -1;
        }
        free(nwc);
        nwc = NULL;
    }
}

struct nwc_connection * get_nwc_connection(struct list_head *head, struct sockaddr *addr)
{
    struct list_head *pos = NULL;
    struct nwc_connection *nwc = NULL;
    struct sockaddr_in *srcaddr = NULL;
    struct sockaddr_in *dstaddr = (struct sockaddr_in*)addr;

    list_for_each(pos, head){
        nwc = list_entry(pos, struct nwc_connection, conn_entry);
        srcaddr = (struct sockaddr_in*)(&nwc->cliaddr);
        if (srcaddr->sin_port == dstaddr->sin_port
                && srcaddr->sin_addr.s_addr == dstaddr->sin_addr.s_addr)
            return nwc;
    }

    return NULL;
}

void add_nwc_tail(struct list_head *head, struct nwc_connection *nwc)
{
    list_add_tail(&nwc->conn_entry, head);
}

void remove_nwc(struct nwc_connection *nwc)
{
    list_del(&nwc->conn_entry);
}
void destroy_all_nwc(struct list_head *hdr)
{
    struct list_head *pos, *n = NULL;
    struct nwc_connection *nwc = NULL;

    list_for_each_safe(pos, n, hdr){
        struct nwc_connection *nwc = list_entry(pos, struct nwc_connection, conn_entry);
        free_nwc_conn(nwc);
        nwc = NULL;
    }
}

