#include <stdlib.h>

#include "nwc_connection.h"

struct nwc_connection *alloc_nwc_conn_arg0()
{
    struct nwc_connection *nwc = (struct nwc_connection*)malloc(sizeof(struct nwc_connection));

    nwc->next = nwc;
    nwc->prev = nwc;
 
    return nwc;
}

struct nwc_connection *alloc_nwc_conn(pid_t pid, int fd)
{
    struct nwc_connection *nwc = (struct nwc_connection*)malloc(sizeof(struct nwc_connection));

    nwc->next = nwc;
    nwc->prev = nwc;
    nwc->pid = pid;
    nwc->fd = fd;

    return nwc;
}

void free_nwc_conn(struct nwc_connection *nwc)
{
    if (nwc){
        if (nwc->recv_buffer){
            free(nwc->recv_buffer);
            nwc->recv_buffer = NULL;
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


