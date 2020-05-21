#include <stdio.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "mcast.h"

#define PORT "13"

int main(int argc, char *argv[])
{
    int rc = 0;
    struct sockaddr_storage grp_addr;
    struct sockaddr_storage peer_addr;
    struct sockaddr_storage local_addr;
    socklen_t addrlen = 0;
    const char *grp_ip = "FF01::1111";
    const char *local_ip = NULL;
    char timeStr[256];
    char letter;
    char clienthost[1024];
    char clientservice[16];

    int sockfd = -1;

    if (argc > 1){
        grp_ip = argv[1];
    }
    if (argc > 2){
        local_ip = argv[2];
    }

    memset(&grp_addr, 0, sizeof(grp_addr));
    memset(&peer_addr, 0, sizeof(peer_addr));
    memset(&local_addr, 0, sizeof(local_addr));

    addrlen = sizeof(peer_addr);

    //install SIGPIPE
    signal(SIGPIPE, SIG_IGN);

    printf("Start mcast client local_ip(%s) mcast_gip(%s) port(%s)\n", local_ip ? local_ip : "", grp_ip, PORT);

    rc = mcast_get_addr(grp_ip, PORT, (struct sockaddr*)&grp_addr);
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
    sockfd = mcast_listen(grp_addr.ss_family, PORT);
    if (sockfd < 0){
        fprintf(stderr, "Open multicast socket failed! errno(%d)\n", errno);
        return -1;
    }

    rc = mcast_join_group(sockfd, (struct sockaddr*)&grp_addr, local_ip ? (struct sockaddr*)&local_addr : NULL);
    if (rc != 0){
        fprintf(stderr, "Join multicast group failed! errno(%d)\n", errno);
        close(sockfd);
        return rc;
    }

    rc = mcast_set_ttl(sockfd, grp_addr.ss_family, 1);
    if (rc != 0){
        fprintf(stderr, "Set multicast ttl failed! rc(%d)\n", errno);
        close(sockfd);
        return rc;
    }
    rc = mcast_set_loop(sockfd, grp_addr.ss_family, 0);
    if (rc != 0){
        fprintf(stderr, "Set multicast loopback failed! rc(%d)\n", errno);
        close(sockfd);
        return rc;
    }

    letter = '1';
    rc = sendto(sockfd, &letter, sizeof(letter), 0, (struct sockaddr*)&grp_addr, sizeof(grp_addr));
    if (rc<0) {
        perror("sendto error::");
        close(sockfd);
        return -1;
    }

    printf("Send message to mcast group(%c)\n", letter);

    memset(timeStr, 0, sizeof(timeStr));
    rc = recvfrom(sockfd, timeStr,
            sizeof(timeStr),
            0,
            (struct sockaddr*)&peer_addr,
            &addrlen);

    if (rc<0) {
        perror("recvfrom error:: ");
        close(sockfd);
        return -1;
    }

    printf("%s\n", timeStr);

    close(sockfd);

    return 0;
}

