#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/un.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "nwchecker.h"

#define RECV_BUFF_LEN 4*1024*1024
#define MAX_EPOLL_EVENTS 1024

int unix_stream_server(struct nwc_args *na)
{
    int rc = -1;
    struct sockaddr_un serveraddr;
    char *recv_buffer = NULL;
    int epfd = -1;
    struct epoll_event ev, events[MAX_EPOLL_EVENTS];

    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1){
        printf("Create unix sream socket failed!errno(%d)\n", errno);
        return -1;
    }

    int flags = fcntl(fd, F_GETFL);
    flags |= O_NONBLOCK; 
    fcntl(fd, F_SETFL, flags);

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

    rc = listen(fd, 5);
    if (rc == -1){
        printf("Listen unix stream server failed!errno(%d)\n", errno);
        close(fd);
        return -1;
    }

    epfd = epoll_create(MAX_EPOLL_EVENTS);
    if (epfd == -1){
        printf("epoll_create failed(%d)\n", errno);
        close(fd);
        return -1;
    }

    ev.events = EPOLLIN;
    ev.data.fd = fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1){
        printf("Add listen fd to epoll failed. errno(%d)\n", errno);
        close(fd);
        close(epfd);
        return -1;
    }

    recv_buffer = alloc_buffer(RECV_BUFF_LEN);
    int output_log = 0;
    while (1){
        int nfds = epoll_wait(epfd, events, MAX_EPOLL_EVENTS, -1);
        if (nfds == -1){
            if (nfds == EINTR)
                continue;
            else
                break;
        }

        for (int i = 0; i < nfds; ++i){
            int sfd = events[i].data.u64 & 0xffffffff;
            int event = events[i].data.u64 >> 32;
            if (sfd == fd){
                int newfd = accept(fd, NULL, NULL);
                if (newfd == -1){
                    printf("Accept new unix stream server failed!fd(%d) errno(%d)\n", fd, errno);
                    if (errno != EINTR){
                        break;
                    }
                }else{
                    printf("Accept a new unix stream fd(%d)\n", newfd);

                    int flags = fcntl(newfd, F_GETFL);
                    flags |= O_NONBLOCK; 
                    fcntl(newfd, F_SETFL, flags);

                    ev.events = EPOLLIN;
                    ev.data.fd = newfd;
                    if (epoll_ctl(epfd, EPOLL_CTL_ADD, newfd, &ev) == -1){
                        printf("Add new fd to epoll failed. errno(%d)\n", errno);
                        close(newfd);
                        break;
                    }
                }
                continue;
            }

            if (events[i].events & EPOLLIN){
                char buff[5] = {0};
                rc = recv(sfd, recv_buffer, RECV_BUFF_LEN, 0);
                if (rc == 0){
                    printf("The fd(%d) closed by peer! errno(%d) rc(%d)\n", sfd, errno, rc);
                    close(sfd);
                    break;
                }

                if (rc == -1){
                    printf("The fd(%d) closed by peer! errno(%d) rc(%d)\n", sfd, errno, rc);
                    if (errno == EINTR || EAGAIN){
                        ;
                    }else{
                        close(sfd);
                        break;
                    }
                }

                output_log = rc <= 5? 1:0;

                if (output_log){
                    printf("Recv (%s) request from(%d)!\n", recv_buffer, sfd);
                }
            }

            if (!event || events[i].events & EPOLLOUT){
                if (events[i].events & EPOLLOUT){
                    ev.events = EPOLLIN;
                    ev.data.u64 = sfd;
                    epoll_ctl(epfd, EPOLL_CTL_MOD, sfd, &ev);
                }
                rc = send(sfd, "pong", 4, 0);
                if (rc == -1){
                    printf("Send pong failed!errno(%d)\n", errno);
                    if (errno == EINTR){
                        ;
                    }else if (errno == EAGAIN){
                        ev.events = EPOLLIN | EPOLLOUT;
                        ev.data.u64 = 2L << 32 | sfd;
                        if (epoll_ctl(epfd, EPOLL_CTL_MOD, sfd, &ev) == -1){
                            printf("Add write event for sfd to epoll failed. errno(%d)\n", errno);
                            close(sfd);
                            break;
                        }
                    }else{
                        close(sfd);
                        break;
                    }
                }

                if (output_log){
                    printf("Send pong response!\n");
                }
            }
        }
    }

    if (recv_buffer){
        free_buffer(recv_buffer);
        recv_buffer = NULL;
    }

    close(fd);

    return 0;
}

