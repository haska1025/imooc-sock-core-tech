#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

#include <sys/socket.h>
#include <arpa/inet.h> // inet_addr()

#include "nwchecker.h"

int nwc_client(struct nwc_args *na)
{
    int rc = 0;
    int sock_fd = -1;
    struct sockaddr_in serveraddr;

    char *send_msg = NULL;
    size_t send_msg_len = 0;

    // Used to receive the "pong" message from the server.
    char recv_buffer[8] = {0};

    sock_fd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sock_fd == -1){
        printf("Create socket failed! errno(%d)\n", errno);
        return -1;
    }

    memset(&serveraddr, 0, sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(na->port);
    serveraddr.sin_addr.s_addr = inet_addr(na->ip);

    rc = connect(sock_fd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (rc == -1){
        printf("Connect server failed! errno(%d)\n", errno);
        return -1;
    }

    send_msg_len = na->message_size <= 0?5:na->message_size;
    send_msg = alloc_buffer(send_msg_len);

    useconds_t sleep_time = na->interval <= 0 ? 3000 : na->interval;
    int slot_send_count = na->sent_pkgs <= 0? 1: na->sent_pkgs;
    int cur_slot_sent = slot_send_count;
    int infinit = na->count <= 0 ? 1:0;
    int loop_count = na->count * slot_send_count;
    int echo_mode = na->echo_mode;

    printf("nwc client. slot_send_count(%d) infinit(%d) loop_count(%d) echo_mode(%d) sleep_time(%d)\n",
            slot_send_count,
            infinit,
            loop_count,
            echo_mode,
            sleep_time);

    while(1){
        rc = send(sock_fd, send_msg, send_msg_len, 0); 
        if (rc == -1){
            printf("Send ping request failed! errno(%d)\n", errno);
            break;
        }

        if (send_msg_len <= 5){
            printf("%ld Send %s to server!\n", time(NULL), send_msg);
        }else{
            printf("%ld Send %ld message to server!\n", time(NULL), send_msg_len);
        }

        if (echo_mode != 2){
            rc = recv(sock_fd, recv_buffer, 7, 0);
            if (rc == 0){
                printf("The connection is closed by peer!\n");
                break;
            }

            if (rc == -1){
                printf("Receive message failed!errno(%d)\n", errno);
                break;
            }

            printf("%ld Recv %s from the server\n", time(NULL), recv_buffer);
        }

        if (!infinit){
            loop_count--;
            if (loop_count == 0){
                break;// exit loop
            }
        }

        // Sleep 3 seconds
        //sleep(3);
        if (cur_slot_sent-- == 0){
            usleep(sleep_time * 1000);
            cur_slot_sent = slot_send_count;
        }
    }

    if (!na->no_close){
        close(sock_fd);
    }

    if (send_msg){
        free_buffer(send_msg);
        send_msg = NULL;
    }

    return 0;
}

