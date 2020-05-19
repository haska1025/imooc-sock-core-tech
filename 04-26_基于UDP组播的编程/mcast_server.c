#include <time.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "mcast.h"

#define PORT "13"

static int mcast_listen(const char *port)
{
    int rc = 0;
    struct addrinfo hints, *res, *ressave;

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
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    rc = getaddrinfo(NULL, port, &hints, &res);
    if (rc != 0){
        printf("mcast_listen get host name failed rc(%d) errstr(%s)", rc, gai_strerror(rc));
        return -1;
    }

    ressave=res;

    rc = -1;

    while (res) {
        int fd = socket(res->ai_family,
                res->ai_socktype,
                res->ai_protocol);

        if (fd >=0 && (res->ai_family == AF_INET || res->ai_family == AF_INET6)) {
            if ((rc = bind(fd, res->ai_addr, res->ai_addrlen)) == 0){
                rc = fd;
                break;
            }
        }

        close(fd);
        res = res->ai_next;
    }

    freeaddrinfo(ressave);

    return rc;
}

int main(int argc, char *argv[])
{
    int rc = 0;
    struct sockaddr_storage grp_addr;
    struct sockaddr_storage peer_addr;
    struct sockaddr_storage local_addr;
    const char *grp_ip = "FF01::1111";
    const char *local_ip = NULL;
    char timeStr[256];
    char b[256];
    time_t now;
    char clienthost[1024];
    char clientservice[16];
    int sockfd = -1;

    if (argc > 1){
        grp_ip = argv[1];
    }
    if (argc > 2){
        local_ip = argv[2];
    }

    //install SIGPIPE
    signal(SIGPIPE, SIG_IGN);

    printf("Start mcast server local_ip(%s) mcast_gip(%s) port(%d)\n", local_ip?local_ip:"", grp_ip, PORT);

    rc = mcast_get_addr(grp_ip, "13", (struct sockaddr*)&grp_addr);
    if (rc != 0){
        fprintf(stderr, "Get multicast addr failed! rc(%d)\n", rc);
        return rc;
    }
    if (local_ip){
        rc = mcast_get_addr(local_ip, NULL, (struct sockaddr*)&local_addr);
        if (rc != 0){
            fprintf(stderr, "Get multicast addr failed! rc(%d)\n", rc);
            return rc;
        }
    }
 
    // Create listen socket
    sockfd = mcast_listen(PORT);
    if (sockfd < 0){
        fprintf(stderr, "Open multicast socket failed! errno(%d)\n", errno);
        return -1;
    }

    rc = mcast_join_group(sockfd, (struct sockaddr*)&grp_addr, local_ip ? (struct sockaddr*)(&local_addr) : NULL);
    if (rc != 0){
        fprintf(stderr, "Join multicast group failed! errno(%d)\n", rc);
        close(sockfd);
        return rc;
    }

    rc = mcast_set_ttl(sockfd, grp_addr.ss_family, 1);
    if (rc != 0){
        fprintf(stderr, "Set multicast ttl failed! rc(%d)\n", rc);
        close(sockfd);
        return rc;
    }
    rc = mcast_set_loop(sockfd, grp_addr.ss_family, 0);
    if (rc != 0){
        fprintf(stderr, "Set multicast loopback failed! rc(%d)\n", rc);
        close(sockfd);
        return rc;
    }

    for ( ; ;) {
        memset(&peer_addr, 0 , sizeof(peer_addr));
        socklen_t socklen = 0;
        socklen = sizeof(peer_addr);

        rc = recvfrom(sockfd,
                b,  
                sizeof(b),
                0,  
                (struct sockaddr*)&peer_addr,
                &socklen);

        if (rc <0) 
            continue;


        memset(timeStr, 0, sizeof(timeStr));
        time(&now);
        sprintf(timeStr, "%s", ctime(&now));

        printf("time str(%s) len(%lu)\n", timeStr, sizeof(timeStr));

        rc = sendto(sockfd, timeStr, sizeof(timeStr), 0, (struct sockaddr*)&grp_addr, sizeof(grp_addr));
        printf("Send messg rc(%d)\n", rc);
    }  

    return 0;
}

