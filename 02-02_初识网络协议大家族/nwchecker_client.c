#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

#include <sys/socket.h>
#include <arpa/inet.h> // inet_addr()

int main(int argc, char *argv[])
{
    int rc = 0;
    int sock_fd = -1;
    struct sockaddr_in serveraddr;

    const char *ping = "ping";
    size_t ping_len = strlen(ping);

    // Used to receive the "pong" message from the server.
    char recv_buffer[8] = {0};


    if (argc < 3){
        printf("Please input: nwchecker_client ip port\n");
        return -1;
    }

    sock_fd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sock_fd == -1){
        printf("Create socket failed! errno(%d)\n", errno);
        return -1;
    }

    memset(&serveraddr, 0, sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(atoi(argv[2]));
    serveraddr.sin_addr.s_addr = inet_addr(argv[1]);

    rc = connect(sock_fd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (rc == -1){
        printf("Connect server failed! errno(%d)\n", errno);
        return -1;
    }

    while(1){
        rc = send(sock_fd, ping, ping_len, 0); 
        if (rc == -1){
            printf("Send ping request failed! errno(%d)\n", errno);
            break;
        }

        printf("%u Send %s to server!\n", time(NULL), ping);

        rc = recv(sock_fd, recv_buffer, 7, 0);
        if (rc == 0){
            printf("The connection is closed by peer!");
            break;
        }

        if (rc == -1){
            printf("Receive message failed!errno(%d)\n", errno);
            break;
        }
        
        printf("%u Recv %s from the server\n", time(NULL), recv_buffer);

        // Sleep 3 seconds
        sleep(3);
    }

    close(sock_fd);

    return 0;
}

