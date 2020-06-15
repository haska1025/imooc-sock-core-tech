#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "nwc_tcp_handler.h"
#include "nwc_sock.h"
#include "nwc.h"

int nwc_tcp_handler_init(struct nwc_tcp_handler *handler)
{
    nwc_io_handler_init(&handler->parent);
    handler->parent.handle_input = nwc_tcp_handle_input;
    handler->parent.handle_output = nwc_tcp_handle_output;
    handler->parent.listen = nwc_tcp_handler_listen;
    handler->parent.connect = nwc_tcp_handler_connect;
    handler->parent.send = nwc_tcp_handler_send;
    handler->parent.recv = nwc_tcp_handler_recv;
    handler->parent.close = nwc_tcp_handler_close;
    handler->state = NTS_CLOSED;

    return 0;
}

int nwc_tcp_handler_listen(struct nwc_io_handler *handler, const char *ip, uint16_t port)
{
    int sock_fd = -1;

    struct nwc_tcp_handler *tcp = tcp_handler(handler);
    
    printf("nwc_tcp_handler_listen start listen ip(%s) port(%d)\n", ip, port);
    sock_fd = nwc_sock_listen(AF_INET, SOCK_STREAM, ip, port);
    if (sock_fd == -1)
        return -1;

    handler->handle_output = NULL;
    handler->fd = sock_fd;
    tcp->state = NTS_LISTEN;

    handler->h_handle = nwc_looper_add_handler(handler->looper, handler);
    nwc_looper_register_event(handler->looper, handler->h_handle, EM_READ);

    return 0;
}

int nwc_tcp_handler_connect(struct nwc_io_handler *handler, const char *hostname, uint16_t port)
{
    int fd = -1;
    int rc = 0;
    int events = EM_READ;

    struct nwc_tcp_handler *tcp = tcp_handler(handler);

    rc= nwc_sock_connect(AF_INET, SOCK_STREAM, hostname, port, &fd);
    if (rc == 0){
        tcp->state = NTS_ESTABLISHED;
    }else if(rc == EINPROGRESS || rc == EWOULDBLOCK){
        events |= EM_WRITE;
        tcp->state = NTS_CONNECTING;
        rc = -EINPROGRESS;
    }else{
        printf("nwc_tcp_handler_connect to server failed! ip(%s) port(%d) rc(%d)\n",
                hostname, port, rc);
        // error
        return rc;
    }

    handler->fd = fd;
    handler->h_handle = nwc_looper_add_handler(handler->looper, handler); 
    nwc_looper_register_event(handler->looper, handler->h_handle, events);

    return rc;
}
int nwc_tcp_handler_accept(struct nwc_tcp_handler *handler, int fd)
{
    handler->parent.fd = fd;
    handler->parent.h_handle = nwc_looper_add_handler(handler->parent.looper, &handler->parent); 
    nwc_looper_register_event(handler->parent.looper, handler->parent.h_handle, EM_READ);
    handler->state = NTS_ESTABLISHED;

    return 0;
}
int nwc_tcp_handler_recv(struct nwc_io_handler *handler, uint8_t buffer[], uint32_t buflen)
{
    int rc = 0;

    while(1){
        int bytes = recv(handler->fd , buffer, buflen , 0 );
        if (bytes == -1){
#ifdef DEBUG
            printf("recv error.fd(%d) errno(%d)\n", handler->fd , errno);
#endif
            if (errno == EINTR)
                continue;
            else if(errno == EAGAIN || errno == EWOULDBLOCK){
                return -EAGAIN;
            }else if(errno == ECONNREFUSED){
                rc = -ECONNREFUSED;
                break;
            }else{
                rc = -1;
                break;
            }
        }

        if(bytes == 0){
            rc = 0;
            break;
        }

        /* recv successfully */
        return bytes;
    }

    printf("Recv error fd(%d) errno(%d) retval(%d)\n",
            handler->fd,
            errno,
            rc);

    return rc;
}

int nwc_tcp_handler_send(struct nwc_io_handler *handler, const uint8_t buffer[], uint32_t buflen)
{
    int rc = 0;

    while(1){
        int bytes = send(handler->fd , (char*)buffer, buflen, 0);
        if (bytes == -1){
            printf("send error fd(%d) errno(%d)\n", handler->fd, errno);
            if (errno == EINTR)
                continue;
            else if(errno == EAGAIN || errno == EWOULDBLOCK){
                handler->can_send = 0;
                rc = -EAGAIN;
                break;
            } else{
                rc = -1;
                break;
            }
        }

        //Send over, maybe just send partition
        return bytes;
    }

    if (rc != -EAGAIN){
        printf("Send error fd(%d) errno(%d) rc(%d)\n",
                handler->fd,
                errno,
                rc);
    }

    return rc;
}
int nwc_tcp_handler_close(struct nwc_io_handler *handler)
{
    struct nwc_tcp_handler *tcp = tcp_handler(handler);

    printf("Close tcphandler(%p) state(%d)\n", handler, tcp->state);

    if (tcp->state == NTS_CLOSED)
        return 0;

    tcp->state = NTS_CLOSED;

    nwc_looper_cancel_event(handler->looper, handler->h_handle, EM_ALL);
    nwc_looper_remove_handler(handler->looper, handler->h_handle);

    close(handler->fd);
    handler->fd = -1;

    return 0;
}

void nwc_tcp_handle_input(struct nwc_io_handler *handler)
{
    struct nwc_tcp_handler *tcp = tcp_handler(handler);

    if(tcp->state == NTS_LISTEN){
        struct sockaddr_storage cliaddr;
        socklen_t cliaddrlen = sizeof(cliaddr);
        memset(&cliaddr, 0, sizeof(cliaddr));

        int newfd = accept(handler->fd, (struct sockaddr*)&cliaddr, &cliaddrlen);
        if (newfd == -1){
            printf("accept new connection failed(%d)\n", errno);
            return;
        }

        if (handler->ioop){
            struct nwc_io_operation *op = NULL;
            struct nwc_tcp_handler *newtcp = malloc(sizeof(struct nwc_tcp_handler));
            nwc_tcp_handler_init(newtcp);
            op = handler->ioop->on_accept(handler->ioop->userdata, (nwc_handle_t)newtcp); 
            if (!op){
                printf("On accept failed!\n");
                free(newtcp);
                newtcp = NULL;
                close(newfd);
                newfd = -1;
                return;
            }

            newtcp->parent.ioop = op;
            newtcp->parent.looper = handler->looper;
            nwc_tcp_handler_accept(newtcp, newfd);
            printf("Accept a new connection(%d:%p)\n", newfd, newtcp);
        }else{
            close(newfd);
            newfd = -1;
        }
    }else{
        if (handler->ioop){
            handler->ioop->on_recv(handler->ioop->userdata);
        }
    }
}
void nwc_tcp_handle_output(struct nwc_io_handler *handler)
{
    int rc = 0;
    struct nwc_tcp_handler *tcp = tcp_handler(handler);

    rc = nwc_looper_cancel_event(handler->looper, handler->h_handle, EM_WRITE);
    printf("cancel event rc(%d)\n", rc);

    if(tcp->state == NTS_CONNECTING){
        int err = 0;
        int len = sizeof(err);
        rc = getsockopt(handler->fd, SOL_SOCKET, SO_ERROR, &err, &len);
        if (rc != 0)
            rc = err;

        if (err != 0){
            if( err == ECONNREFUSED ||
                    err == ECONNRESET ||
                    err == ETIMEDOUT ||
                    err == EHOSTUNREACH ||
                    err == ENETUNREACH ||
                    err == ENETDOWN ||
                    err == EINVAL){
                printf("Connect failed. errno(%d)\n", errno);
                return;
            }
        }
        // connection ok
        tcp->state = NTS_ESTABLISHED; 
        if (handler->ioop){
            handler->ioop->on_connect(handler->ioop->userdata);
        }
    }else{
        handler->can_send = 1;
        if (handler->ioop){
            handler->ioop->on_send(handler->ioop->userdata);
        }
    }
}

