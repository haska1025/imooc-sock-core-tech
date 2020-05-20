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

    printf("Start mcast server local_ip(%s) mcast_gip(%s) port(%s)\n", local_ip?local_ip:"", grp_ip, PORT);

    rc = mcast_get_addr(grp_ip, PORT, (struct sockaddr*)&grp_addr);
    if (rc != 0){
        fprintf(stderr, "Get multicast addr failed! rc(%d)\n", rc);
        return rc;
    }

    mcast_inet_ntop((struct sockaddr*)&grp_addr, clienthost, 255);

    printf("The group ip(%s)\n", clienthost);

    if (local_ip){
        rc = mcast_get_addr(local_ip, NULL, (struct sockaddr*)&local_addr);
        if (rc != 0){
            fprintf(stderr, "Get multicast addr failed! rc(%d)\n", rc);
            return rc;
        }
    }
 
    // Create listen socket
    sockfd = mcast_listen(grp_addr.ss_family, PORT);
    if (sockfd < 0){
        fprintf(stderr, "Open multicast socket failed! errno(%d)\n", errno);
        return -1;
    }

    rc = mcast_join_group(sockfd, (struct sockaddr*)&grp_addr, local_ip ? (struct sockaddr*)(&local_addr) : NULL);
    if (rc != 0){
        fprintf(stderr, "Join multicast group failed! errno(%d)\n", errno);
        close(sockfd);
        return rc;
    }

    rc = mcast_set_ttl(sockfd, grp_addr.ss_family, 1);
    if (rc != 0){
        fprintf(stderr, "Set multicast ttl failed! errno(%d)\n", errno);
        close(sockfd);
        return rc;
    }
    rc = mcast_set_loop(sockfd, grp_addr.ss_family, 0);
    if (rc != 0){
        fprintf(stderr, "Set multicast loopback failed! errno(%d)\n", errno);
        close(sockfd);
        return rc;
    }

    for ( ; ;) {
        memset(&peer_addr, 0 , sizeof(peer_addr));
        socklen_t addrlen = sizeof(peer_addr);

        rc = recvfrom(sockfd,
                b,  
                sizeof(b),
                0,  
                (struct sockaddr*)&peer_addr,
                &addrlen);

        if (rc <0) 
            continue;

        b[rc] = '\0';
        printf("Recv message(%s)\n", b);

        memset(timeStr, 0, sizeof(timeStr));
        time(&now);
        sprintf(timeStr, "%s", ctime(&now));


        rc = sendto(sockfd, timeStr, sizeof(timeStr), 0, (struct sockaddr*)&grp_addr, sizeof(grp_addr));
        printf("Send message time str(%s) len(%lu) rc(%d)\n", timeStr, sizeof(timeStr), rc);
    }  

    return 0;
}

