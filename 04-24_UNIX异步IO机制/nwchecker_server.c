#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h> // inet_addr()

#include "nwchecker.h"
#include "nwc_connection.h"

#define RECV_BUFF_LEN 4*1024*1024

struct nwc_connection nwc_hdr;

int nwc_server(struct nwc_args *na)
{
    int rc = 0;
    int sock_fd = -1;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len;
    fd_set readfds, writefds;
    struct timeval timeout, *timeout_ptr = NULL;

    // Used to receive the "ping" message from the server.
    int client_sock_fd = -1;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sock_fd == -1){
        printf("Create socket failed! errno(%d)\n", errno);
        return -1;
    }

    int flags = fcntl(sock_fd, F_GETFL);
    flags |= O_NONBLOCK; 
    fcntl(sock_fd, F_SETFL, flags);


    int enable = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) == -1){
        printf("setsockopt(SO_REUSEADDR) failed errno(%d)", errno);
        close(sock_fd);
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
        close(sock_fd);
        return -1;
    }

    rc = listen(sock_fd, 3);
    if (rc == -1){
        printf("Server listen failed! errno(%d)\n", errno);
        close(sock_fd);
        return -1;
    }

    INIT_NWC(&nwc_hdr);

    int echo_mode = na->echo_mode;
    useconds_t sleep_time = na->interval < 0 ? 0:na->interval;

    pid_t pid = -1;

    while(1){
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);

        FD_SET(sock_fd, &readfds);
        int max_fd = sock_fd;

        for (struct nwc_connection *nwc = nwc_hdr.next; nwc != &nwc_hdr;){
            if (nwc->fd == -1){
                struct nwc_connection *nwc_next = nwc->next;
                struct nwc_connection *nwc_prev = nwc->prev;
                nwc_next->prev = nwc_prev;
                nwc_prev->next = nwc_next;
                free_nwc_conn(nwc);
                nwc = nwc_next;
            }else{
                if (nwc->events & 1){
                    FD_SET(nwc->fd, &readfds);
                }
                if (nwc->events & 2){
                    FD_SET(nwc->fd, &writefds);
                }

                if (max_fd < nwc->fd){
                    max_fd = nwc->fd;
                }
                nwc = nwc->next;
            }
        }

        rc = select(max_fd + 1, &readfds, &writefds, NULL, timeout_ptr);
        if (rc == -1){
            printf("Select error(%d)\n",errno);
            if (errno == EINTR){
                continue;
            }
            break;
        }

        if (FD_ISSET(sock_fd, &readfds)){
            client_addr_len = sizeof(client_addr);
            client_sock_fd = accept(sock_fd, (struct sockaddr*)&client_addr, (socklen_t*)&client_addr_len);
            if (client_sock_fd == -1){
                printf("Accept connection from client failed! errno(%d)\n", errno);
                if (errno != EINTR){
                    break;
                }
            }else{
                printf("Accept new connection(%d) from client(%s:%d)\n", 
                        client_sock_fd,
                        inet_ntoa(client_addr.sin_addr),
                        ntohs(client_addr.sin_port));

                struct nwc_connection *nwc = alloc_nwc_conn(pid, client_sock_fd);
                nwc->events = 1;// Add read event
                nwc->recv_buffer = alloc_buffer(RECV_BUFF_LEN);

                int flags = fcntl(client_sock_fd, F_GETFL);
                flags |= O_NONBLOCK; 
                fcntl(client_sock_fd, F_SETFL, flags);

                nwc->events = 1;
                nwc->fd = client_sock_fd;
                add_nwc_tail(&nwc_hdr, nwc);
            }
        }

        for (struct nwc_connection *nwc = nwc_hdr.next; nwc != &nwc_hdr; nwc = nwc->next){
            int output_log = 1;
            if (FD_ISSET(nwc->fd, &readfds)){
                if (echo_mode != 3){
                    rc = recv(nwc->fd, nwc->recv_buffer, RECV_BUFF_LEN, 0);
                    if (rc == 0){
                        printf("The connection is closed by peer!\n");
                        close(nwc->fd);
                        nwc->fd = -1;
                        break;
                    }

                    if (rc == -1){
                        printf("Receive message failed!errno(%d)\n", errno);
                        if (errno == EINTR){
                            ;
                        }else if (errno == EAGAIN){
                            ;// do nothing
                        }else{
                            close(nwc->fd);
                            nwc->fd = -1;
                            break;
                        }
                    }

                    output_log = rc <= 5? 1:0;
                    if (output_log){
                        printf("%ld Recv %s from the client(%d)\n",
                                time(NULL),
                                nwc->recv_buffer,
                                nwc->fd);
                    }
                }
            }

            if (!(nwc->events & 2) || FD_ISSET(nwc->fd, &writefds)){
                nwc->events &= 1;
                if (echo_mode != 2){
                    const char *pong = "pong";
                    size_t pong_len = strlen(pong);
                    rc = send(nwc->fd, pong, pong_len, 0); 
                    if (rc == -1){
                        printf("Send ping reponse failed! errno(%d)\n", errno);
                        if (errno == EINTR){
                            ;
                        }else if (errno == EAGAIN){
                            nwc->events |= 2;
                        }else{
                            close(nwc->fd);
                            nwc->fd = -1;
                            break;
                        }
                    }

                    if (output_log){
                        printf("%ld Send %s to client !\n", time(NULL), pong);
                    }
                }
            }
        }

        if (sleep_time > 0){
            timeout.tv_sec = sleep_time / 1000;
            timeout.tv_usec = (sleep_time % 1000) * 1000;
            timeout_ptr = &timeout;
        }else{
            timeout_ptr = NULL;
        }
    }

    destroy_all_nwc(&nwc_hdr);
    close(sock_fd);

    return 0;
}

