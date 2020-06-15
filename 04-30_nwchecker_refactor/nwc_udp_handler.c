#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "nwc_udp_handler.h"
#include "nwc_sock.h"
#include "nwc.h"

#define UDP_SERVER 1
#define UDP_CLIENT 2

int nwc_udp_handler_init(struct nwc_udp_handler *handler)
{
    nwc_io_handler_init(&handler->parent);
    handler->parent.handle_input = nwc_udp_handle_input;
    handler->parent.handle_output = nwc_udp_handle_output;

    handler->parent.listen = nwc_udp_handler_listen;
    handler->parent.connect = nwc_udp_handler_connect;
    handler->parent.send = nwc_udp_handler_send;
    handler->parent.sendto = nwc_udp_handler_sendto;
    handler->parent.recv = nwc_udp_handler_recv;
    handler->parent.recvfrom = nwc_udp_handler_recvfrom;
    handler->parent.close = nwc_udp_handler_close;

    return 0;
}

int nwc_udp_handler_listen(struct nwc_io_handler *handler, const char *ip, uint16_t port)
{
    int sock_fd = -1;
    struct nwc_udp_handler *udp = udp_handler(handler);

    printf("nwc_udp_handler_listen start listen ip(%s) port(%d)\n", ip, port);
    sock_fd = nwc_sock_listen(AF_INET, SOCK_DGRAM, ip, port);
    if (sock_fd == -1)
        return -1;

    handler->handle_output = NULL;
    handler->fd = sock_fd;

    handler->h_handle = nwc_looper_add_handler(handler->looper, handler);
    nwc_looper_register_event(handler->looper, handler->h_handle, EM_READ);

    udp->type = UDP_SERVER;

    return 0;
}

int nwc_udp_handler_connect(struct nwc_io_handler *handler, const char *hostname, uint16_t port)
{
    int fd = -1;
    int rc = 0;
    int events = EM_READ;
    struct nwc_udp_handler *udp = udp_handler(handler);

    printf("nwc_udp_handler_connect connect to server ip(%s) port(%d)\n", hostname, port);

    rc= nwc_sock_connect(AF_INET, SOCK_DGRAM, hostname, port, &fd);
    if (rc != 0){
        printf("udp sock create failed!rc(%d) errno(%d)\n", rc, errno);
        return rc;
    }

    handler->fd = fd;
    handler->h_handle = nwc_looper_add_handler(handler->looper, handler); 
    nwc_looper_register_event(handler->looper, handler->h_handle, events);

    udp->type = UDP_CLIENT;

    return rc;
}

int nwc_udp_handler_send(struct nwc_io_handler *handler, const uint8_t buffer[], uint32_t buflen)
{
    return nwc_udp_handler_sendto(handler, buffer, buflen, NULL, 0);
}
int nwc_udp_handler_recv(struct nwc_io_handler *handler, uint8_t buffer[], uint32_t buflen)
{
    return nwc_udp_handler_recvfrom(handler, buffer, buflen, NULL, 0);
}
int nwc_udp_handler_recvfrom(struct nwc_io_handler *handler, uint8_t buffer[], uint32_t buflen, struct sockaddr *fromaddr, socklen_t *fromaddrlen)
{
    int rc = 0;

    while(1){
        int bytes = recvfrom(handler->fd , buffer, buflen , 0, fromaddr, fromaddrlen);
        if (bytes == -1){
#ifdef DEBUG
            printf("recvfrom error.fd(%d) errno(%d)\n", handler->fd , errno);
#endif
            if (errno == EINTR)
                continue;
            else if(errno == EAGAIN || errno == EWOULDBLOCK){
                return -EAGAIN;
            }else{
                rc = -1;
                break;
            }
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

int nwc_udp_handler_sendto(struct nwc_io_handler *handler, const uint8_t buffer[], uint32_t buflen, struct sockaddr *toaddr, socklen_t toaddrlen)
{
    int rc = 0;

    while(1){
        int bytes = sendto(handler->fd , (char*)buffer, buflen, 0, toaddr, toaddrlen);
        if (bytes == -1){
            printf("send error fd(%d) errno(%d)\n", handler->fd, errno);
            if (errno == EINTR)
                continue;
            else if(errno == EAGAIN || errno == EWOULDBLOCK){
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
int nwc_udp_handler_close(struct nwc_io_handler *handler)
{
    printf("Close udphandler(%p)", handler);

    nwc_looper_cancel_event(handler->looper, handler->h_handle, EM_ALL);
    nwc_looper_remove_handler(handler->looper, handler->h_handle);

    close(handler->fd);
    handler->fd = -1;

    return 0;
}

void nwc_udp_handle_input(struct nwc_io_handler *handler)
{
    struct nwc_udp_handler *udp = udp_handler(handler);

    if(udp->type == UDP_SERVER){
        if (handler->ioop){
            handler->ioop->on_accept(handler->ioop->userdata, (nwc_handle_t)handler);
        }
    }else{
        if (handler->ioop){
            handler->ioop->on_recv(handler->ioop->userdata);
        }
    }
}
void nwc_udp_handle_output(struct nwc_io_handler *handler)
{
    int rc = 0;
    struct nwc_udp_handler *udp = udp_handler(handler);

    rc = nwc_looper_cancel_event(handler->looper, handler->h_handle, EM_WRITE);
    printf("cancel event rc(%d)\n", rc);
    handler->can_send = 1;
    if (handler->ioop){
        handler->ioop->on_send(handler->ioop->userdata);
    }
}


