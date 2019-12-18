#include <stdlib.h>

#include "nwc_connection.h"

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
        free(nwc);
        nwc = NULL;
    }
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


