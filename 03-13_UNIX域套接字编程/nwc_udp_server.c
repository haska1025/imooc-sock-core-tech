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

int nwc_udp_server(struct nwc_args *na)
{
    int rc = 0;
    int sock_fd = -1;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len;
    char *recv_buffer = NULL;
    size_t recv_buffer_len =  RECV_BUFF_LEN;

    const char *pong = "pong";
    size_t pong_len = strlen(pong);

    // Used to receive the "ping" message from the server.
    int client_sock_fd = -1;

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0); 
    if (sock_fd == -1){
        printf("Create socket failed! errno(%d)\n", errno);
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

    INIT_NWC(&nwc_hdr);

    int echo_mode = na->echo_mode;
    useconds_t sleep_time = na->interval < 0 ? 0:na->interval;

    recv_buffer = alloc_buffer(RECV_BUFF_LEN);

    while(1){
        client_addr_len = sizeof(client_addr);
        if (echo_mode != 3){
            rc = recvfrom(sock_fd, recv_buffer, RECV_BUFF_LEN, 0, (struct sockaddr*)&client_addr, &client_addr_len);
            if (rc == -1){
                printf("Receive message failed!errno(%d)\n", errno);
                continue;
            }

            printf("%ld Recv %s from the client(%s:%d)\n",
                    time(NULL),
                    recv_buffer,
                    inet_ntoa(client_addr.sin_addr),
                    ntohs(client_addr.sin_port));
        }

        if (sleep_time > 0){
            usleep(sleep_time * 1000);
        }

        if (echo_mode != 2){
            rc = sendto(sock_fd, pong, pong_len, 0, (struct sockaddr*)&client_addr, client_addr_len); 
            if (rc == -1){
                printf("Send ping reponse failed! errno(%d)\n", errno);
                continue;
            }
            printf("%ld Send %s to client(%s:%d) !\n",
                    time(NULL),
                    pong,
                    inet_ntoa(client_addr.sin_addr),
                    ntohs(client_addr.sin_port));
        }

        struct nwc_connection *nwc = get_nwc_connection(&nwc_hdr, (struct sockaddr*)&client_addr); 
        if (!nwc){
            printf("Create new nwc connection for client(%s:%d)\n",
                    inet_ntoa(client_addr.sin_addr),
                    ntohs(client_addr.sin_port));

            nwc = alloc_nwc_conn_arg0();
            nwc->addr.sin_port = client_addr.sin_port;
            nwc->addr.sin_addr = client_addr.sin_addr;
            add_nwc_tail(&nwc_hdr, nwc);
        }else{
            // The connection exists, use it.
        }
    }

    if (!na->no_close){
        close(sock_fd);
    }

    if (recv_buffer){
        free_buffer(recv_buffer);
        recv_buffer = NULL;
    }

    destroy_all_nwc(&nwc_hdr);

    return 0;
}

