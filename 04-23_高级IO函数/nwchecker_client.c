#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h> // inet_addr()

#include "nwchecker.h"

#define CONN_UNCONNECTED 1
#define CONN_CONNECTED 2

int nwc_client(struct nwc_args *na)
{
    int rc = 0;
    int sock_fd = -1;
    struct sockaddr_in serveraddr;

    char *send_msg = NULL;
    size_t send_msg_len = 0;

    struct timeval timeout, *timeout_ptr = NULL;
    fd_set readfds, writefds;
    int events = 1;// 1: read, 2: write, 3, read and write.
    int state = CONN_UNCONNECTED;// 1: unconnected, 2: connected.

    // Used to receive the "pong" message from the server.
    char recv_buffer[8] = {0};

    sock_fd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sock_fd == -1){
        printf("Create socket failed! errno(%d)\n", errno);
        return -1;
    }

    int flags = fcntl(sock_fd, F_GETFL);
    flags |= O_NONBLOCK; 
    fcntl(sock_fd, F_SETFL, flags);

    memset(&serveraddr, 0, sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(na->port);
    serveraddr.sin_addr.s_addr = inet_addr(na->ip);

    rc = connect(sock_fd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (rc == -1){
        rc = errno;
        if (rc != EINPROGRESS && rc != EWOULDBLOCK){
            printf("Connect server failed! errno(%d)\n", errno);
            close(sock_fd);
            return -1;
        }
        events |= 2;
    }else{
        // connection ok
        state = CONN_CONNECTED; 
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;
        timeout_ptr = &timeout;
        events &= 0x1;
        printf("Connection success...1\n");
    }

    send_msg_len = na->message_size <= 0?5:na->message_size;
    send_msg = alloc_buffer(send_msg_len);

    useconds_t sleep_time = na->interval <= 0 ? 3000 : na->interval;
    int slot_send_count = na->sent_pkgs <= 0? 1: na->sent_pkgs;
    int cur_slot_sent = slot_send_count;
    int infinit = na->count <= 0 ? 1:0;
    int loop_count = na->count * slot_send_count;
    int echo_mode = na->echo_mode;

    printf("nwc client. slot_send_count(%d) infinit(%d) loop_count(%d) echo_mode(%d) sleep_time(%d) rc(%d)\n",
            slot_send_count,
            infinit,
            loop_count,
            echo_mode,
            sleep_time,
            rc);

    unsigned long total_send_size = 0;
    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    while(1){
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);

        if (events & 1){
            FD_SET(sock_fd, &readfds);
        }

        if (events & 2){
            FD_SET(sock_fd, &writefds);
        }

        rc = select(sock_fd+1, &readfds, &writefds, NULL, timeout_ptr);
        if (rc == -1){
            printf("Select error(%d)\n",errno);
            if (errno == EINTR){
                continue;
            }
            break;
        }

        if (state == CONN_UNCONNECTED){
            if (FD_ISSET(sock_fd, &writefds)){
                int err = 0;
                int len = sizeof(err);
                rc = getsockopt(sock_fd, SOL_SOCKET, SO_ERROR, &err, &len);
                if (rc != 0)
                    rc = err;

                if (err != 0){
                    if( err == ECONNREFUSED ||
                            err == ECONNRESET ||
                            err == ETIMEDOUT ||
                            err == EHOSTUNREACH ||
                            err == ENETUNREACH ||
                            err == ENETDOWN ||
                            err == EINVAL){
                        printf("Connect failed. errno(%d)\n", errno);
                        break;
                    }
                }
                // connection ok
                state = CONN_CONNECTED; 
                timeout_ptr = &timeout;
                events &= 0x1;
                printf("Connection success...2!\n");
            }
        }

        if (state == CONN_CONNECTED){
            if (!(events & 2) || FD_ISSET(sock_fd, &writefds)){
                events &= 0x1;
                rc = send(sock_fd, send_msg, send_msg_len, 0); 
                if (rc == -1){
                    printf("Send ping request failed! errno(%d)\n", errno);
                    if (errno == EINTR){
                        continue;
                    }else if (errno == EAGAIN){
                        events |= 2;
                    }else{
                        break;
                    }
                }
                total_send_size += send_msg_len;
                if (send_msg_len <= 5){
                    printf("%ld Send %s to server!\n", time(NULL), send_msg);
                }
            }


            if (FD_ISSET(sock_fd, &readfds)){
                rc = recv(sock_fd, recv_buffer, 7, 0);
                if (rc == 0){
                    printf("The connection is closed by peer!\n");
                    break;
                }

                if (rc == -1){
                    printf("Receive message failed!errno(%d)\n", errno);
                    if (errno == EINTR){
                        continue;
                    }else if (errno == EAGAIN){
                        ;// do nothing
                    }else{
                        break;
                    }
                }

                if (send_msg_len <= 5){
                    printf("%ld Recv %s from the server\n", time(NULL), recv_buffer);
                }
            }
        }

        if (!infinit){
            loop_count--;
            if (loop_count == 0){
                break;// exit loop
            }
        }

        // Sleep 3 seconds
        cur_slot_sent--; 
        if (cur_slot_sent == 0){
            cur_slot_sent = slot_send_count;
            timeout.tv_sec = sleep_time / 1000;
            timeout.tv_usec = (sleep_time % 1000) * 1000;
        }else{
            timeout.tv_sec = 0;
            timeout.tv_usec = 0;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    unsigned long secs = (end.tv_sec - start.tv_sec) + (end.tv_nsec-end.tv_nsec)/1000000000;
    unsigned long bytes_per_sec = total_send_size;

    if (secs > 0){
        bytes_per_sec = total_send_size/secs;
    }

    printf("Send totalsize(%lu) secs(%lu) bytes_per_sec(%lu)\n",
            total_send_size,
            secs,
            bytes_per_sec);

    if (!na->no_close){
        close(sock_fd);
    }

    if (send_msg){
        free_buffer(send_msg);
        send_msg = NULL;
    }

    return 0;
}

