#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h> // inet_addr()

#include "nwchecker.h"
#include "nwc_connection.h"

#define RECV_BUFF_LEN 4*1024*1024

struct nwc_connection nwc_hdr;

void worker_process(struct nwc_connection *nwc)
{
    int rc = 0;

    char *recv_buffer = NULL;

    const char *pong = "pong";
    size_t pong_len = strlen(pong);

    recv_buffer = alloc_buffer(RECV_BUFF_LEN);

    int output_log = 0;

    while(1){
        if (nwc->echo_mode != 3){
            rc = recv(nwc->fd, recv_buffer, RECV_BUFF_LEN, 0);
            if (rc == 0){
                printf("The connection is closed by peer!\n");
                break;
            }

            if (rc == -1){
                printf("Receive message failed!errno(%d)\n", errno);
                break;
            }

            output_log = rc <= 5? 1:0;

            if (output_log){
                printf("%ld Recv %s from the client(%d)\n",
                        time(NULL),
                        recv_buffer,
                        nwc->fd);
            }
        }

        if ( nwc->sleep_time > 0){
            usleep(nwc->sleep_time * 1000);
        }

        if ( nwc->echo_mode != 2){
            rc = send(nwc->fd, pong, pong_len, 0); 
            if (rc == -1){
                printf("Send ping reponse failed! errno(%d)\n", errno);
                break;
            }

            if (output_log){
                printf("%ld Send %s to client !\n", time(NULL), pong);
            }
        }
    }

    close(nwc->fd);

    if (recv_buffer){
        free_buffer(recv_buffer);
        recv_buffer = NULL;
    }

    printf("The child(%d) fd(%d) exit\n", getpid(), nwc->fd);

    _exit(1);
}


int nwc_server(struct nwc_args *na)
{
    int rc = 0;
    int sock_fd = -1;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len;


    // Used to receive the "ping" message from the server.
    int client_sock_fd = -1;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sock_fd == -1){
        printf("Create socket failed! errno(%d)\n", errno);
        return -1;
    }

    int enable = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) == -1){
        printf("setsockopt(SO_REUSEADDR) failed errno(%d)", errno);
        return -1;
    }

    memset(&client_addr, 0, sizeof(client_addr));
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(na->port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    rc = bind(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (rc == -1){
        printf("Bind server failed! errno(%d)\n", errno);
        return -1;
    }

    rc = listen(sock_fd, 3);
    if (rc == -1){
        printf("Server listen failed! errno(%d)\n", errno);
        return -1;
    }

    INIT_NWC(&nwc_hdr);

    int echo_mode = na->echo_mode;
    useconds_t sleep_time = na->interval < 0 ? 0:na->interval;

    pid_t pid = -1;

    while(1){
        client_addr_len = sizeof(client_addr);
        client_sock_fd = accept(sock_fd, (struct sockaddr*)&client_addr, (socklen_t*)&client_addr_len);
        if (client_sock_fd == -1){
            printf("Accept connection from client failed! errno(%d)\n", errno);
            return -1;
        }

        printf("Accept new connection(%d) from client(%s:%d)\n", 
                client_sock_fd,
                inet_ntoa(client_addr.sin_addr),
                ntohs(client_addr.sin_port));

        struct nwc_connection *nwc = alloc_nwc_conn(pid, client_sock_fd);
        nwc->echo_mode = echo_mode;
        nwc->sleep_time = sleep_time;

        pid = fork();
        if (pid == -1){
            printf("fork child process failed.errno(%d)\n",errno);
            free_nwc_conn(nwc);
            nwc = NULL;

            continue;
        }else if(pid == 0){
            // Child process
            worker_process(nwc);
        }else{
            // Parent process
            nwc->pid = pid;
            close(client_sock_fd);
            add_nwc_tail(&nwc_hdr, nwc);
        }
    }

    if (!na->no_close){
        close(sock_fd);
    }

    return 0;
}

