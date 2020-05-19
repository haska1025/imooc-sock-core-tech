#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "mcast.h"

static int mcast_join_or_leave_group(int fd, struct sockaddr *mcast_addr, struct sockaddr *localaddr, int isjoin)
{
    int optname;
    int rc = 0;

    if(mcast_addr->sa_family == AF_INET){
        struct ip_mreq      mreq;
        mreq.imr_multiaddr.s_addr= ((struct sockaddr_in *)mcast_addr)->sin_addr.s_addr;
        if (localaddr){
            mreq.imr_interface.s_addr = ((struct sockaddr_in *)localaddr)->sin_addr.s_addr;
        } else{
            mreq.imr_interface.s_addr = INADDR_ANY;
        }

        optname = isjoin? IP_ADD_MEMBERSHIP : IP_DROP_MEMBERSHIP;
        rc = setsockopt(fd, IPPROTO_IP, optname, &mreq, sizeof(mreq));
    } else if(mcast_addr->sa_family == AF_INET6) {
        struct ipv6_mreq    mreq6;

        memcpy(&mreq6.ipv6mr_multiaddr, &(((struct sockaddr_in6 *)mcast_addr)->sin6_addr), sizeof(struct in6_addr));
        if (localaddr){
            mreq6.ipv6mr_interface = ((struct sockaddr_in6 *)localaddr)->sin6_scope_id; // cualquier interfaz
        } else {
            mreq6.ipv6mr_interface = 0;
        }

        optname = isjoin? IPV6_JOIN_GROUP: IPV6_LEAVE_GROUP;
        rc = setsockopt(fd, IPPROTO_IPV6, optname, &mreq6, sizeof(mreq6));
    }

    return rc;
}

int mcast_get_addr(const char *hostname, const char *service, struct sockaddr *addr)
{
    struct addrinfo hints, *res, *ressave;
    int rc, sockfd;

    rc = -1;
    res = NULL;
    ressave = NULL;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    rc = getaddrinfo(hostname, service, &hints, &res);
    if (rc != 0){
        printf("get host name failed rc(%d) errstr(%s)\n", rc, gai_strerror(rc));
        return -1;
    }

    ressave = res;

    sockfd=-1;
    while (res) {
        sockfd = socket(res->ai_family,
                res->ai_socktype,
                res->ai_protocol);
        if (!(sockfd < 0)) {
            close(sockfd);
            sockfd=-1;
            memcpy(addr, res->ai_addr, res->ai_addrlen);
            rc = 0;
            break;
        }
        res=res->ai_next;
    }

    freeaddrinfo(ressave);
    return rc;
}
int mcast_join_group(int fd, struct sockaddr *mcast_addr, struct sockaddr *localaddr)
{
    return mcast_join_or_leave_group(fd, mcast_addr, localaddr, 1);
}
int mcast_leave_group(int fd, struct sockaddr *mcast_addr, struct sockaddr *localaddr)
{
    return mcast_join_or_leave_group(fd, mcast_addr, localaddr, 0);
}
int mcast_set_ttl(int fd, int family, int ttl)
{
    int rc = 0;

    if (family == AF_INET){
        rc = setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
    }else if(family == AF_INET6){
        rc = setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &ttl, sizeof(ttl));
    }
    return rc;
}
int mcast_set_loop(int fd, int family, int flag)
{
    int rc = 0;
    if (family == AF_INET){
        rc = setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &flag, sizeof(flag));
    }else if(family == AF_INET6){
        rc = setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &flag, sizeof(flag));
    }
    return rc;
}

