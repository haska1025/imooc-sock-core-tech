#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "nwchecker.h"

#define RECV_BUFF_LEN 4*1024*1024

int unix_stream_server(struct nwc_args *na)
{
    int rc = -1;
    struct sockaddr_un serveraddr;
    char *recv_buffer = NULL;

    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1){
        printf("Create unix sream socket failed!errno(%d)\n", errno);
        return -1;
    }

    memset(&serveraddr, 0, sizeof(serveraddr));

    serveraddr.sun_family = AF_UNIX;
    strncpy(serveraddr.sun_path, na->ip, sizeof(serveraddr.sun_path)-1);
    unlink(serveraddr.sun_path);

    rc = bind(fd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (rc == -1){
        printf("Bind unix stream server failed!errno(%d)\n", errno);
        return -1;
    }

    rc = listen(fd, 5);
    if (rc == -1){
        printf("Listen unix stream server failed!errno(%d)\n", errno);
        return -1;
    }

    recv_buffer = alloc_buffer(RECV_BUFF_LEN);

    int output_log = 0;

    while (1){
        int newfd = accept(fd, NULL, NULL);
        if (newfd == -1){
            printf("Accept new unix stream server failed!fd(%d) errno(%d)\n", fd, errno);
            break;
        }

        printf("Accept a new unix stream fd(%d)\n", fd);

        if (fork() == 0){
            close(fd);
            while(1){
                char buff[5] = {0};
                rc = recv(newfd, recv_buffer, RECV_BUFF_LEN, 0);
                if (rc <= 0){
                    printf("Recv ping failed!errno(%d) rc(%d)\n", errno, rc);
                    break;
                }

                output_log = rc <= 5? 1:0;

                if (output_log){
                    printf("Recv (%s) request!\n", buff);
                }
                rc = send(newfd, "pong", 4, 0);
                if (rc == -1){
                    printf("Send pong failed!errno(%d)\n", errno);
                    break;
                }

                if (output_log){
                    printf("Send pong response!\n");
                }
            }
            close(newfd);
            _exit(0);
        }else{
            close(newfd);
        }
    }

    if (recv_buffer){
        free_buffer(recv_buffer);
        recv_buffer = NULL;
    }

    close(fd);

    return 0;
}

