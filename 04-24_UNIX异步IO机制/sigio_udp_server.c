#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>

#include "nwchecker.h"
#include "nwc_connection.h"

#define RECV_BUFF_LEN 4*1024*1024

struct nwc_connection nwc_hdr;
int g_sockfd = -1;

static void sigio_handler(int signo)
{
    while(1){
        struct nwc_connection *nwc = alloc_nwc_conn_arg0();

        memset(&nwc->addr, 0, sizeof(nwc->addr)); 
        nwc->addr_len = sizeof(nwc->addr);
        nwc->recv_buffer = malloc(RECV_BUFF_LEN);
        int rc = recvfrom(g_sockfd, nwc->recv_buffer, RECV_BUFF_LEN, 0, (struct sockaddr*)&nwc->addr, &nwc->addr_len);
        if (rc == -1){
            free_nwc_conn(nwc);
            nwc = NULL;
            return;
        }

        add_nwc_tail(&nwc_hdr, nwc);
    }
}

int sigio_udp_server(struct nwc_args *na)
{
    int rc = 0;
    int sock_fd = -1;
    struct sockaddr_in server_addr;

    const char *pong = "pong";
    size_t pong_len = strlen(pong);

    // Used to receive the "ping" message from the server.
    struct sigaction sa;
    sigset_t io_sigset, old_sigset;

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0); 
    if (sock_fd == -1){
        printf("Create socket failed! errno(%d)\n", errno);
        return -1;
    }

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

    INIT_NWC(&nwc_hdr);

    g_sockfd = sock_fd;
    // Set signal handler
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = sigio_handler;
    sa.sa_flags = 0;
    rc = sigaction(SIGIO, &sa, NULL);
    if (rc == -1){
        printf("Seit sigio failed errno(%d)\n", errno);
        return -1;
    }

    // Set ownpid
    fcntl(sock_fd, F_SETOWN, getpid());

    // Set nonblocking and async
    int flags = fcntl(sock_fd, F_GETFL);
    flags = flags | O_NONBLOCK | O_ASYNC; 
    fcntl(sock_fd, F_SETFL, flags);

    // Set signal mask
    sigemptyset(&io_sigset);
    sigemptyset(&old_sigset);
    sigaddset(&io_sigset, SIGIO);
    sigprocmask(SIG_BLOCK, &io_sigset, &old_sigset);

    while(1){
        if (nwc_hdr.next == &nwc_hdr)
            sigsuspend(&old_sigset);

        for (struct nwc_connection *nwc = nwc_hdr.next; nwc != &nwc_hdr; ){
            printf("%ld Recv %s from the client(%s:%d)\n",
                    time(NULL),
                    nwc->recv_buffer,
                    inet_ntoa(nwc->addr.sin_addr),
                    ntohs(nwc->addr.sin_port));

            rc = sendto(sock_fd, pong, pong_len, 0, (struct sockaddr*)&nwc->addr, nwc->addr_len); 
            printf("%ld Send %s to client(%s:%d)! rc_error(%d:%d)\n",
                    time(NULL),
                    pong,
                    inet_ntoa(nwc->addr.sin_addr),
                    ntohs(nwc->addr.sin_port),
                    rc,
                    errno);
            struct nwc_connection *nwc_next = nwc->next;
            struct nwc_connection *nwc_prev = nwc->prev;
            nwc_next->prev = nwc_prev;
            nwc_prev->next = nwc_next;
            free_nwc_conn(nwc);
            nwc = nwc_next;
        }
    }

    if (!na->no_close){
        close(sock_fd);
    }

    destroy_all_nwc(&nwc_hdr);

    return 0;
}

