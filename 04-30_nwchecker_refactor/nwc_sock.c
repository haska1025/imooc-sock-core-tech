#include "nwc_sock.h"

#include <netdb.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>

int nwc_sock_create(int family, int socktype, int protocol, int nonblock, int reuseaddr)
{
    int fd = -1;
    int rc = 0;

    fd = socket(family,
            socktype,
            protocol);
    if (fd == -1){
        printf("create socket failed family(%d) socktype(%d) protocol(%d) errno(%d)\n",
                family,
                socktype,
                protocol,
                errno);
        return -1;
    }

    if (nonblock){
        int flags = fcntl(fd, F_GETFL);
        flags |= O_NONBLOCK; 
        fcntl(fd, F_SETFL, flags);
    }

    if (reuseaddr){
        int enable = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) == -1){
            printf("setsockopt(SO_REUSEADDR) failed errno(%d)", errno);
            close(fd);
            return -1;
        }
    }

    return fd;
}

int nwc_sock_listen(int family, int socktype, const char *host, uint16_t port)
{
    int rc = 0;
    int fd = -1;
    struct addrinfo hints, *res, *ressave;
    char ports[16] = {0};

    sprintf(ports, "%u", port);
    memset(&hints, 0, sizeof(struct addrinfo));

    /*
     * AI_PASSIVE flag: the resulting address is used to bind
     * to a socket for accepting incoming connections.
     * So, when the hostname==NULL, getaddrinfo function will
     * return one entry per allowed protocol family containing
     * the unspecified address for that family.
     */
    res = NULL;
    ressave = NULL;
    hints.ai_flags    = AI_PASSIVE;
    hints.ai_family   = family;
    hints.ai_socktype = socktype;

    rc = getaddrinfo(host, ports, &hints, &res);
    if (rc != 0){
        printf("nwc sock listen get host name failed rc(%d) errstr(%s)", rc, gai_strerror(rc));
        return -1;
    }

    ressave=res;

    rc = -1;

    while (res) {
        if (res->ai_socktype ==socktype && res->ai_family == family) {
            fd = nwc_sock_create(res->ai_family,
                    res->ai_socktype,
                    res->ai_protocol,
                    1,
                    1);
            if (fd >= 0){
                if ((rc = bind(fd, res->ai_addr, res->ai_addrlen)) == 0){
                    rc = fd;
                    break;
                }
                close(fd);
                fd = -1;
            }
        }
        res = res->ai_next;
    }

    freeaddrinfo(ressave);

    if (rc < 0){
        printf("nwc_sock_listen create socket(%d) failed(%d)\n", rc , errno);
        return rc;
    }

    if (rc >= 0 && socktype == SOCK_STREAM ){
        if(listen(rc, 5) == -1){
            printf("nwc_sock_listen listen socket(%d) failed(%d)\n", rc , errno);
            close(rc);
            return -1;
        }
    }

    return rc;
}
int nwc_sock_connect(int family, int socktype, const char *peerhost, uint8_t peerport, int *fd)
{
    int rc=0;
    struct addrinfo hints, *res, *ressave;
    char ports[16] = {0};
    int sock_fd = -1;

    sprintf(ports, "%u", peerport);
    res = NULL;
    ressave = NULL;
    memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_family = family;
    hints.ai_socktype = socktype;

    rc = getaddrinfo(peerhost, ports, &hints, &res);
    if (rc != 0){
        printf("nwc_sock_connect get host name failed rc(%d) errstr(%s)", rc, gai_strerror(rc));
        return -1;
    }
    ressave = res;
    while (res) {
        /* the socket is nonblocked and async connect */
        sock_fd = nwc_sock_create( res->ai_family,
                res->ai_socktype,
                res->ai_protocol,
                1,
                0);
        if (sock_fd >= 0){
            rc = connect(sock_fd, res->ai_addr, res->ai_addrlen);
            if (rc == 0 || errno == EINPROGRESS || errno == EWOULDBLOCK){
                rc = rc==0 ? 0: errno;
                *fd = sock_fd;
                break;
            }

            close(sock_fd);
        }
        res = res->ai_next;
    }

    freeaddrinfo(ressave);

    return rc;
}

