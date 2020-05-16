#include <stdlib.h>

#include "nwc_connection.h"

struct nwc_io_op * alloc_nwc_io_op(int op)
{
    struct nwc_io_op *ioop = (struct nwc_io_op*)malloc(sizeof(struct nwc_io_op));
    ioop->op = op;
    ioop->socket = INVALID_SOCKET;
    memset(&ioop->olapped, 0, sizeof(ioop->olapped));
    return ioop;
}
void free_nwc_io_op(struct nwc_io_op *op)
{
    if (op) {
        free(op);
    }
}

/////////////////// The nwc connection ////////////////////////////////
struct nwc_connection *alloc_nwc_conn_arg0()
{
    struct nwc_connection *nwc = (struct nwc_connection*)malloc(sizeof(struct nwc_connection));

    nwc->next = nwc;
    nwc->prev = nwc;
    nwc->socket = INVALID_SOCKET;
    nwc->rbytes = 0;

    nwc->wsabuf.buf = nwc->recv_buff;
    nwc->wsabuf.len = RECV_BUFFER_LEN;

    return nwc;
}


void free_nwc_conn(struct nwc_connection *nwc)
{
    if (nwc){
        if (nwc->socket != INVALID_SOCKET) {
            closesocket(nwc->socket);
            nwc->socket = INVALID_SOCKET;
        }
        free(nwc);
        nwc = NULL;
    }
}

struct nwc_connection * get_nwc_connection(struct nwc_connection *hdr, struct sockaddr *addr)
{
    struct nwc_connection *nwc = hdr->next;
    struct sockaddr_in *dstaddr = (struct sockaddr_in*)addr;

    for (; nwc != hdr;nwc = nwc->next){
        if (nwc->addr.sin_port == dstaddr->sin_port
                && nwc->addr.sin_addr.s_addr == dstaddr->sin_addr.s_addr)
            return nwc;
    }

    return NULL;
}

void add_nwc_tail(struct nwc_connection *hdr, struct nwc_connection *nwc)
{
    struct nwc_connection *prev = hdr->prev;
    struct nwc_connection *next = hdr;
    nwc->prev = prev;
    nwc->next = next;
    prev->next = nwc;
    next->prev = nwc;
}

void remove_nwc(struct nwc_connection *nwc)
{
    struct nwc_connection *prev = nwc->prev;
    struct nwc_connection *next = nwc->next;

    prev->next = next;
    next->prev = prev;
}
void destroy_all_nwc(struct nwc_connection *hdr)
{
    struct nwc_connection *nwc = hdr->next;
    for (; nwc != hdr;){
        struct nwc_connection *next = nwc->next;
        free_nwc_conn(nwc);
        nwc = next;
    }
}


