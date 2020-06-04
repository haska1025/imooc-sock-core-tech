#include <stdio.h>
//#include <sys/epoll.h>
#include <errno.h>
#include <stdlib.h>

#include "nwc_epoll_looper.h"
#include "nwc_types.h"
#include "nwc_io_handler.h"

#ifndef MAX_EPOLL_WAIT_EVENTS
#define MAX_EPOLL_WAIT_EVENTS 4000
#endif


static int __nwc_epoll_ctl(struct nwc_epoll_looper *looper, struct epoll_entry *entry , int op)
{
    int rc;
    int events = 0;
    if (entry->mask & EM_READ){
        events |= EPOLLIN;
    }

    if(entry->mask & EM_WRITE){
        events |= EPOLLOUT;
    }

    struct epoll_event ev;
    ev.data.ptr = entry;
    ev.events = events;

    rc = epoll_ctl(looper->epfd , op , entry->fd, &ev);

    return (rc == -1)? errno:0;
}

int nwc_epoll_start(struct nwc_epoll_looper *looper)
{
    looper->epfd = epoll_create(MAX_EPOLL_WAIT_EVENTS);
    if (looper->epfd == -1){
        printf("epoll instance create failed! errno(%d)\n", errno);
        return errno;
    }

    looper->exit = 0;
    printf("Initialize epoll successfully epfd(%d)\n", looper->epfd);

    return 0;
}
int nwc_epoll_add_handler(struct nwc_epoll_looper *looper, struct nwc_io_handler *handler)
{
    int rc = 0;
    struct epoll_entry *entry;

    entry = malloc(sizeof(struct epoll_entry));
    if (!entry)
        return INVALID_HANDLE;

    entry->fd = handler->fd;
    entry->eh = handler;
    entry->mask = 0;
    rc = __nwc_epoll_ctl(looper, entry, EPOLL_CTL_ADD);
    if (rc != 0){
        /* register failed and this is the first register ,delete the entry */
        free(entry);
        entry = NULL;
        return INVALID_HANDLE;
    }

    return (nwc_handle_t)entry;
}
int nwc_epoll_remove_handler(struct nwc_epoll_looper *looper,struct nwc_handle_t* handle)
{
    int rc;
    struct epoll_entry *entry = (struct epoll_entry*)handle;

    entry->mask = 0;
    rc = __nwc_epoll_ctl(looper, entry, EPOLL_CTL_DEL);
    entry->fd = INVALID_HANDLE;
    list_add_tail(&entry->remove_entry, &looper->removed_epoll_entrys);

    return rc;
}
int nwc_epoll_register_event(struct nwc_epoll_looper *looper,struct nwc_handle_t* handle, int events)
{
    struct epoll_entry *entry = (struct epoll_entry*)handle;
    /* save new flags */
    entry->mask |= flags;
    return __nwc_epoll_ctl(looper, entry, EPOLL_CTL_MOD);
}
int nwc_epoll_cancel_event(struct nwc_epoll_looper *looper,struct nwc_handle_t* handle, int events)
{
    struct epoll_entry *entry = (struct epoll_entry*)handle;
    entry->mask &= (~flags);
    return __nwc_epoll_ctl(looper, entry, EPOLL_CTL_MOD);
}
void nwc_epoll_run(struct nwc_epoll_looper *looper)
{
    struct epoll_event ev_list[MAX_EPOLL_WAIT_EVENTS];
    struct list_head *pos = NULL, *n = NULL;

    printf("enter epoll loop timer_interval(%d)", looper->timeout_interval);
    while(!looper->exit){
        int readys = 0;
        /*
         * Dispatch timer event.
         * We must dispatch tiemr events firstly before the i/o events.
         */

        readys = epoll_wait(looper->epfd , ev_list , MAX_EPOLL_WAIT_EVENTS , looper->timeout_interval);
        if (readys == -1){
            if (errno == EINTR)
                continue;
            else{
                printf("epoll_wait exit! errno(%d)\n" , errno);
                return -1;
            }
        }

        for (int i = 0; i < readys; ++i){
            struct epoll_entry *entry = NULL;

            entry = (struct epoll_entry*)ev_list[i].data.ptr;

            if (entry->fd == INVALID_HANDLE)
                continue;
#ifdef _DEBUG
            printf("handler = %p, fd = %p , mask=%d,  events=%d\n",
                    entry->eh,
                    (void*)entry->fd,
                    entry->mask,
                    ev_list[i].events);
#endif

            if (ev_list[i].events & (EPOLLHUP | EPOLLERR|EPOLLRDHUP))
                entry->eh->handle_input();

            if (entry->fd == INVALID_HANDLE)
                continue;
            if ( ev_list[i].events & EPOLLIN)
                entry->eh->handle_input();

            if (entry->fd == INVALID_HANDLE)
                continue;
            if (ev_list[i].events & EPOLLOUT)
                entry->eh->handle_output();
        }

        // Remove all removed epoll entrys
        list_for_each_safe(pos, n, &looper->removed_epoll_entrys){
            struct epoll_entry *entry = list_entry(pos, struct epoll_entry, remove_entry);
            list_del(pos);
            free(entry);
            entry = NULL;
        }
    }

    // exit loop
    close(looper->epfd);

    return 0;
}

int nwc_epoll_stop(struct nwc_epoll_looper *looper)
{
    looper->exit = 1;
    return 0;
}
