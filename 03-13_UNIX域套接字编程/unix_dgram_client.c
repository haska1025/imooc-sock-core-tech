#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "nwchecker.h"

int unix_dgram_client(struct nwc_args *na)
{
    int rc = -1;
    struct sockaddr_un serveraddr, localaddr;

    int fd = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (fd == -1){
        printf("Create unix dgram socket failed!errno(%d)\n", errno);
        return -1;
    }

    memset(&localaddr, 0, sizeof(localaddr));
    localaddr.sun_family = AF_LOCAL;
    strncpy(localaddr.sun_path, na->localaddr, sizeof(localaddr.sun_path) - 1);

    rc = bind(fd, (struct sockaddr*)&localaddr, sizeof(localaddr));
    if (rc == -1){
        printf("Bind unix local dgram addr failed!errno(%d)\n", errno);
        return -1;
    }

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sun_family = AF_LOCAL;
    strncpy(serveraddr.sun_path, na->ip, sizeof(serveraddr.sun_path) - 1);

    if (na->is_udp_connect){
        rc = connect(fd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
        if (rc == -1){
            printf("Connect unix dgram server failed!errno(%d)\n", errno);
            return -1;
        }
    }

    while(1){
        if (na->is_udp_connect){
            rc = sendto(fd, "ping", 4, 0, NULL , 0);
        }else{
            rc = sendto(fd, "ping", 4, 0, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
        }
        if (rc == -1){
            printf("Send ping failed!errno(%d)\n", errno);
            break;
        }

        printf("Send ping request!\n");
        char buff[5] = {0};

        struct sockaddr_un peeraddr;
        socklen_t peeraddrlen = sizeof(peeraddr);
        memset(&peeraddr, 0, sizeof(peeraddr));

        if (na->is_udp_connect){
            rc = recvfrom(fd, buff, 4, 0, NULL, 0);
        }else{
            rc = recvfrom(fd, buff, 4, 0, (struct sockaddr*)&peeraddr, &peeraddrlen);
        }
        if (rc <= 0){
            printf("Recv pong failed!errno(%d) rc(%d)\n", errno, rc);
            break;
        }

        printf("Recv (%s) response!\n", buff);
    }

    return 0;
}


