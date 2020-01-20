#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

void child_worker(int fd)
{
    srand(fd);
    unsigned char num = 1;
    while(num++){
        sleep((rand()% 5)+1);
        write(fd, &num, sizeof(num));
    }
    close(fd);
}

int main(int argc, char *argv)
{
    int sock_fds[3][2] = {-1};
    fd_set readfds;
    int worker_exit[3] = {0};
    pid_t worker_pids[3] = {0};


    for (int i=0; i < 3; ++i){
        if (socketpair(AF_LOCAL, SOCK_STREAM, 0, sock_fds[i]) == -1){
            printf("Invoke socketpair failed(%d)\n", errno);
            return -1;
        }
    }

    for (int i=0; i < 3; ++i){
        pid_t pid = fork();
        if (pid == 0){
            close(sock_fds[i][1]);
            child_worker(sock_fds[i][0]);
        }else{
            worker_pids[i] = pid;
            close(sock_fds[i][0]);
        }
    }

    while(1){
        int maxfd = -1;
        FD_ZERO(&readfds);
        for (int i=0; i < 3; ++i){
            FD_SET(sock_fds[i][1], &readfds);
            if (sock_fds[i][1] > maxfd)
                maxfd = sock_fds[i][1];
        }

        int nfds = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        if (nfds == -1){
            printf("Select error(%d)\n", errno);
            if (errno == EINTR){
                continue;
            }

            break;
        }

        for (int i=0; i < 3; ++i){
            if (FD_ISSET(sock_fds[i][1], &readfds)){
                int num = 0;
                read(sock_fds[i][1], &num, sizeof(num));
                printf("Master recv worker[%d:%d]: %d\n", worker_pids[i], sock_fds[i][1], num);
                if (num == 255){
                   worker_exit[i] = 1; 
                }
            }
        }

        int exit_num=0;
        for (;exit_num < 3; ++exit_num){
            if (!worker_exit[exit_num])
                break;
        }
        // exit loop
        if (exit_num == 3)
            break;
    }

    for (int i = 0; i < 3; ++i){
        waitpid(worker_pids[i], NULL, WUNTRACED);
        close(sock_fds[i][1]);
    }

    return 0;
}

