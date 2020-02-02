#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "nwchecker.h"

int unix_dgram_server(struct nwc_args *na)
{
    int rc = -1;
    struct sockaddr_un serveraddr, peeraddr;
    socklen_t peeraddrlen;

    int fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (fd == -1){
        printf("Create unix dgram socket failed!errno(%d)\n", errno);
        return -1;
    }

    memset(&serveraddr, 0, sizeof(serveraddr));

    serveraddr.sun_family = AF_UNIX;
    strncpy(serveraddr.sun_path, na->ip, sizeof(serveraddr.sun_path)-1);
    unlink(serveraddr.sun_path);

    rc = bind(fd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (rc == -1){
        printf("Bind unix stream server failed!errno(%d)\n", errno);
        close(fd);
        return -1;
    }


    while (1){
        char buff[5] = {0};
        memset(&peeraddr, 0, sizeof(peeraddr));
        peeraddrlen = sizeof(peeraddr);
        rc = recvfrom(fd, buff, 4, 0, (struct sockaddr*)&peeraddr, &peeraddrlen);
        if (rc <= 0){
            printf("Recv ping failed!errno(%d) rc(%d)\n", errno, rc);
            continue;
        }

        printf("Recv (%s) request!\n", buff);
        rc = sendto(fd, "pong", 4, 0, (struct sockaddr*)&peeraddr, peeraddrlen);
        if (rc == -1){
            printf("Send pong failed!errno(%d)\n", errno);
            continue;
        }

        printf("Send pong response!\n");
    }

    close(fd);

    return 0;
}

