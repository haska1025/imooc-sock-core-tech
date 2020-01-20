#include <poll.h>
/* According to earlier standards */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

void worker(int fd)
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
    int worker_exit[3] = {0};
    pid_t worker_pids[3] = {0};
    struct pollfd fds[3];


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
            worker(sock_fds[i][0]);
        }else{
            worker_pids[i] = pid;
            close(sock_fds[i][0]);
        }
    }


    while(1){
        for (int i=0; i < 3; ++i){
            fds[i].fd = sock_fds[i][1];
            fds[i].events = POLLIN;
            fds[i].revents = 0;
        }

        int nfds = poll(fds, 3, -1);
        if (nfds == -1){
            printf("Poll error(%d)\n", errno);
            break;
        }

        for (int i=0; i < 3; ++i){
            if (fds[i].revents & POLLIN){
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

