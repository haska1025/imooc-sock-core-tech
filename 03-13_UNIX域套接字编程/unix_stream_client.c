#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "nwchecker.h"

int unix_stream_client(struct nwc_args *na)
{
    int rc = -1;
    struct sockaddr_un serveraddr;
    char *send_msg = NULL;
    size_t send_msg_len = 0;

    unsigned long total_send_size = 0;

    int fd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (fd == -1){
        printf("Create unix sream socket failed!errno(%d)\n", errno);
        return -1;
    }

    memset(&serveraddr, 0, sizeof(serveraddr));

    serveraddr.sun_family = AF_LOCAL;
    strncpy(serveraddr.sun_path, na->ip, sizeof(serveraddr.sun_path) - 1);

    rc = connect(fd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (rc == -1){
        printf("Connect unix stream server failed!errno(%d)\n", errno);
        return -1;
    }

    useconds_t sleep_time = na->interval <= 0 ? 3000 : na->interval;
    int slot_send_count = na->sent_pkgs <= 0? 1: na->sent_pkgs;
    int cur_slot_sent = slot_send_count;
    int infinit = na->count <= 0 ? 1:0;
    int loop_count = na->count * slot_send_count;

    send_msg_len = na->message_size <= 0?5:na->message_size;
    send_msg = alloc_buffer(send_msg_len);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    while(1){
        rc = send(fd, send_msg, send_msg_len, 0);
        if (rc == -1){
            printf("Send ping failed!errno(%d)\n", errno);
            break;
        }

        total_send_size += send_msg_len;

        if (send_msg_len <= 5){
            printf("Send ping request!\n");
        }
        char buff[5] = {0};
        rc = recv(fd, buff, 4, 0);
        if (rc <= 0){
            printf("Recv pong failed!errno(%d) rc(%d)\n", errno, rc);
            break;
        }

        if (send_msg_len <= 5){
            printf("Recv (%s) response!\n", buff);
        }

        if (!infinit){
            loop_count--;
            if (loop_count == 0){
                break;// exit loop
            }
        }

        cur_slot_sent--; 
        if (cur_slot_sent == 0){
            usleep(sleep_time * 1000);
            cur_slot_sent = slot_send_count;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    unsigned long secs = (end.tv_sec - start.tv_sec) + (end.tv_nsec-end.tv_nsec)/1000000000;
    unsigned long bytes_per_sec = total_send_size;

    if (secs > 0){
        bytes_per_sec = total_send_size/secs;
    }

    printf("Send totalsize(%lu) secs(%lu) bytes_per_sec(%lu)",
            total_send_size,
            secs,
            bytes_per_sec);

    return 0;
}


