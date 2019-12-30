#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>

struct sockopt_set
{
    char *optname;
    int level;
    int opt;
};

struct sockopt_set sockopts[] = {
    {"SO_ACCEPTCONN", SOL_SOCKET, SO_ACCEPTCONN},
    {"SO_BROADCAST", SOL_SOCKET, SO_BROADCAST},
    {"SO_DEBUG", SOL_SOCKET, SO_DEBUG},
    {"SO_DONTROUTE", SOL_SOCKET, SO_DONTROUTE},
    {"SO_ERROR", SOL_SOCKET, SO_ERROR},
    {"SO_KEEPALIVE", SOL_SOCKET, SO_KEEPALIVE},
    {"SO_OOBINLINE", SOL_SOCKET, SO_OOBINLINE},
    {"SO_RCVBUF", SOL_SOCKET, SO_RCVBUF},
    {"SO_RCVLOWAT", SOL_SOCKET, SO_RCVLOWAT},
    {"SO_REUSEADDR", SOL_SOCKET, SO_REUSEADDR},
    {"SO_SNDBUF",SOL_SOCKET, SO_SNDBUF},
    {"SO_SNDLOWAT", SOL_SOCKET, SO_SNDLOWAT},
    {"SO_TYPE", SOL_SOCKET, SO_TYPE},
    
    {"IP_MULTICAST_LOOP", IPPROTO_IP, IP_MULTICAST_LOOP},
    {"IP_MULTICAST_TTL", IPPROTO_IP, IP_MULTICAST_TTL},
    {"IP_TTL", IPPROTO_IP, IP_TTL},
   
    {"IPV6_MULTICAST_LOOP", IPPROTO_IPV6, IPV6_MULTICAST_LOOP},
    {"IPV6_MULTICAST_HOPS", IPPROTO_IPV6, IPV6_MULTICAST_HOPS},
#ifdef TCP_DEFER_ACCEPT 
    {"TCP_DEFER_ACCEPT", IPPROTO_TCP, TCP_DEFER_ACCEPT},
#endif
#ifdef TCP_QUICKACK
    {"TCP_QUICKACK", IPPROTO_TCP, TCP_QUICKACK},
#endif
#ifdef TCP_CORK
    {"TCP_CORK", IPPROTO_TCP, TCP_CORK},
#endif
    {"TCP_NODELAY", IPPROTO_TCP, TCP_NODELAY},
#ifdef UDP_CORK
    {"UDP_CORK ", IPPROTO_UDP, UDP_CORK}
#endif
    {NULL, 0, 0}
};

void show_int_value(int socktype)
{
    int ENTRYS = sizeof(sockopts)/ sizeof(struct sockopt_set);

    for (int i=0; i < ENTRYS; ++i){
        if (sockopts[i].optname == NULL)
            continue;

        int pf = AF_INET;
        if (sockopts[i].level == IPPROTO_IPV6){
            pf = AF_INET6;
        }

        int fd = socket(pf, socktype, 0);
        if (fd == -1){
            printf("invoke socket failed errno(%d) errstr(%s)\n", errno, strerror(errno));
            continue;
        }

        int optval = 0;
        int optlen = sizeof(optval);
        int rc = getsockopt(fd, sockopts[i].level, sockopts[i].opt, &optval, &optlen);
        if (rc == 0){
            printf("(%s) level(%d) opt(%d) optval(%d)\n",
                    sockopts[i].optname,
                    sockopts[i].level,
                    sockopts[i].opt,
                    optval);
        }else{
            printf("Getsockopt(%s) failed errno(%d) errstr(%s)\n",
                    sockopts[i].optname,
                    errno,
                    strerror(errno));
        }
    }
}


int main(int argc, char *argv[])
{
    printf("Test Stream ######################\n");
    // Test Stream
    show_int_value(SOCK_STREAM);

    printf("\n\nTest Dgram ######################\n");
    // Test Dgram
    show_int_value(SOCK_DGRAM);

    return 0;
}


