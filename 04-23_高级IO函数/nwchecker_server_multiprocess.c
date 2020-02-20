#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <arpa/inet.h> // inet_addr()

#include "nwchecker.h"
#include "nwc_connection.h"

#define RECV_BUFF_LEN 4*1024*1024
#define MAX_NR_OF_DESCRIPTORS 1

#define CMD_EXIT 1
#define CMD_FD 2

struct worker_cmd
{
    int cmd;
    char *msg;
};

struct worker_context
{
    pid_t pid;
    // Master to worker
    int notify_fd[2];

    int listen_fd;
    int timeout;
    struct nwc_connection nwc_hdr;
};

static int current_worker_index = 0;
static struct worker_context worker_ctxs[4];

static int worker_context_init(struct worker_context *ctx, int timeout)
{
    if (socketpair(AF_LOCAL, SOCK_STREAM, 0, ctx->notify_fd) == -1){
        printf("Invoke socketpair failed(%d)\n", errno);
        return -1;
    }

    ctx->pid = -1;
    ctx->listen_fd = -1;
    ctx->timeout = timeout;

    INIT_NWC(&ctx->nwc_hdr);

    return 0;
}


// Get worker by Round Robin policy.
static struct worker_context *get_worker()
{
    current_worker_index %= 4;
    return &worker_ctxs[current_worker_index++];
}

// sendmsg wrapper
static int worker_send_msg(int sockfd, struct iovec *iov, int iovlen,const int *fds, int fdnum)
{
    char buf[CMSG_SPACE(sizeof(int) * MAX_NR_OF_DESCRIPTORS)];
    struct msghdr msg;
    struct cmsghdr *cmsg = NULL;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = iovlen;
    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);

    cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int) * fdnum);
    memcpy(CMSG_DATA(cmsg), fds, sizeof(int) *fdnum);

    return sendmsg(sockfd, &msg, 0);
}

// recvmsg wrapper
static int worker_recv_msg(int sockfd, struct iovec *iov, int iovlen, int *fds, int *fdnum)
{
    char buf[CMSG_SPACE(sizeof(int) * MAX_NR_OF_DESCRIPTORS)];
    struct msghdr msg;
    struct cmsghdr *cmsg = NULL;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = iovlen;
    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);

    int rc = recvmsg(sockfd, &msg, 0);
    if (rc < 0) return -errno;

    cmsg = CMSG_FIRSTHDR(&msg);
    if (!cmsg){
        *fdnum = 0;
        return rc;
    }

    for (;cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg)){
        if (cmsg->cmsg_type == SCM_RIGHTS){
            int payload = cmsg->cmsg_len - sizeof(struct cmsghdr);
            int desc_nr = payload/sizeof(int);
            memcpy(fds, CMSG_DATA(cmsg), sizeof(int) *desc_nr);
            *fdnum = desc_nr;
        }
    }

    return rc;
}

static void worker(struct worker_context *ctx)
{
    fd_set readfds, writefds;
    struct timeval timeout, *timeout_ptr = NULL;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len;
    int client_sock_fd = -1;
    int max_fd = -1;
    int rc = -1;

    ctx->pid = getpid();

    while(1){
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);

        if (ctx->notify_fd[1] != -1){
            FD_SET(ctx->notify_fd[1], &readfds);
            max_fd = ctx->notify_fd[1];
        }


        if (ctx->listen_fd != -1){
            FD_SET(ctx->listen_fd, &readfds);
            if (ctx->listen_fd > max_fd)
                max_fd = ctx->listen_fd;
        }


        for (struct nwc_connection *nwc = ctx->nwc_hdr.next; nwc != &ctx->nwc_hdr;){
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

        if (ctx->notify_fd[1] != -1 && FD_ISSET(ctx->notify_fd[1], &readfds)){
            // Read 
            struct iovec io[2];
            int cmd = -1;
            char buf[8] = {0};

            io[0].iov_base = &cmd;
            io[0].iov_len = 4;
            io[1].iov_base = buf;
            io[1].iov_len = 7;

            int fd, fdnum;
            fd = -1;
            fdnum =1;
            worker_recv_msg(ctx->notify_fd[1], io, 2, &fd, &fdnum); 
            
            printf("recv new cmd(%d:%s) fd(%d) from master.\n", cmd, buf, fd);

            struct nwc_connection *nwc = alloc_nwc_conn(ctx->pid, fd);
            nwc->events = 1;// Add read event
            nwc->recv_buffer = alloc_buffer(RECV_BUFF_LEN);

            int flags = fcntl(fd, F_GETFL);
            flags |= O_NONBLOCK; 
            fcntl(fd, F_SETFL, flags);

            nwc->events = 1;
            nwc->fd = fd;
            add_nwc_tail(&ctx->nwc_hdr, nwc);
        }

        if (FD_ISSET(ctx->listen_fd, &readfds)){
            memset(&client_addr, 0, sizeof(client_addr));
            client_addr_len = sizeof(client_addr);
            client_sock_fd = accept(ctx->listen_fd, (struct sockaddr*)&client_addr, (socklen_t*)&client_addr_len);
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

                // Notify worker
                struct worker_context *ctx = get_worker();
                struct iovec io[2];
                int cmd = CMD_FD; 
                char *msg = "fd";
                io[0].iov_base = &cmd;
                io[0].iov_len = 4;
                io[1].iov_base = msg;
                io[1].iov_len = 2;
                worker_send_msg(ctx->notify_fd[0], io, 2, &client_sock_fd, 1);
                close(client_sock_fd);
           }
        }

        for (struct nwc_connection *nwc = ctx->nwc_hdr.next; nwc != &ctx->nwc_hdr; nwc = nwc->next){
            int output_log = 1;
            if (FD_ISSET(nwc->fd, &readfds)){
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

                printf("%ld Recv %s from the client(%d)\n",
                        time(NULL),
                        nwc->recv_buffer,
                        nwc->fd);
            }

            if (!(nwc->events & 2) || FD_ISSET(nwc->fd, &writefds)){
                nwc->events &= 1;
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

                printf("%ld Send %s to client !\n", time(NULL), pong);
            }
        }

        if (ctx->timeout > 0){
            timeout.tv_sec = ctx->timeout / 1000;
            timeout.tv_usec = (ctx->timeout % 1000) * 1000;
            timeout_ptr = &timeout;
        }else{
            timeout_ptr = NULL;
        }
    }

    destroy_all_nwc(&ctx->nwc_hdr);
    close(ctx->notify_fd[1]);
}

int nwc_server_process(struct nwc_args *na)
{
    int rc = 0;
    int sock_fd = -1;
    struct sockaddr_in server_addr;

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


    useconds_t sleep_time = na->interval < 0 ? 0:na->interval;

    for (int i=0; i < 4; i++){
        pid_t pid = fork();
        if (pid == 0){
            if (worker_context_init(&worker_ctxs[i], sleep_time) != 0){
                exit(-1);
            }

            close(worker_ctxs[i].notify_fd[0]);
            worker_ctxs[i].notify_fd[0] = -1;
            worker(&worker_ctxs[i]);
        }else{
            close(worker_ctxs[i].notify_fd[1]);
            worker_ctxs[i].notify_fd[1] = -1;
            worker_ctxs[i].pid = pid;
        }
    }

    struct worker_context ctx;

    ctx.notify_fd[0] = -1;
    ctx.notify_fd[1] = -1;
    ctx.pid = getpid();
    ctx.listen_fd = sock_fd;
    ctx.timeout = 0;

    worker(&ctx);

    for (int i=0; i < 4; i++){
        waitpid(worker_ctxs[i].pid, NULL, WUNTRACED);
        close(worker_ctxs[i].notify_fd[0]);
    }

    close(sock_fd);

    return 0;
}

