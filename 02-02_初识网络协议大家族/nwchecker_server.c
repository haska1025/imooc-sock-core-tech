#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h> // inet_addr()

#include "nwchecker.h"

int nwc_server(struct nwc_args na);
{
    int rc = 0;
    int sock_fd = -1;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len;

    const char *pong = "pong";
    size_t pong_len = strlen(pong);

    // Used to receive the "ping" message from the server.
    char recv_buffer[8] = {0};
    int client_sock_fd = -1;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0); 
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

    rc = listen(sock_fd, 3);
    if (rc == -1){
        printf("Server listen failed! errno(%d)\n", errno);
        return -1;
    }

    client_addr_len = sizeof(client_addr);

    client_sock_fd = accept(sock_fd, (struct sockaddr*)&client_addr, (socklen_t*)&client_addr_len);
    if (client_sock_fd == -1){
        printf("Accept connection from client failed! errno(%d)\n", errno);
        return -1;
    }

    while(1){
        rc = recv(client_sock_fd, recv_buffer, 7, 0);
        if (rc == 0){
            printf("The connection is closed by peer!");
            break;
        }

        if (rc == -1){
            printf("Receive message failed!errno(%d)\n", errno);
            break;
        }

        printf("%ld Recv %s from the client\n", time(NULL), recv_buffer);
        
        rc = send(client_sock_fd, pong, pong_len, 0); 
        if (rc == -1){
            printf("Send ping reponse failed! errno(%d)\n", errno);
            break;
        }

        printf("%ld Send %s to client !\n", time(NULL), pong);
        
    }

    if (!na->no_close){
        close(sock_fd);
        close(client_sock_fd);
    }

    return 0;
}

