#ifndef _NWC_CONNECTION_H_
#define _NWC_CONNECTION_H_

#include <sys/types.h>

struct nwc_connection
{
    struct nwc_connection *next;
    struct nwc_connection *prev;

    pid_t pid;
    int fd;
    int echo_mode;
    int sleep_time;
};

#define INIT_NWC(nwc)\
    do { \
        (nwc)->next = (nwc);\
        (nwc)->prev = (nwc);\
        (nwc)->pid = 0;\
        (nwc)->fd = -1;\
    }while(0);

struct nwc_connection *alloc_nwc_conn(pid_t pid, int fd);
void free_nwc_conn(struct nwc_connection *nwc);

void add_nwc_tail(struct nwc_connection *hdr, struct nwc_connection *nwc);
void remove_nwc(struct nwc_connection *nwc); 
void destroy_all_nwc(struct nwc_connection *hdr);


#endif//_NWC_CONNECTION_H_

