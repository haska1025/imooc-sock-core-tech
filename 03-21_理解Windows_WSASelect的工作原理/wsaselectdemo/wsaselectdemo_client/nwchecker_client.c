#include <Windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>


int main(int argc, char *argv[])
{
    int rc = 0;
    SOCKET sock_fd = -1;
    struct sockaddr_in serveraddr;

    char *send_msg = "ping";
    int send_msg_len = 4;

    // Used to receive the "pong" message from the server.
    char recv_buffer[8] = {0};
    WSADATA wsaData = { 0 };
	unsigned short port = 800;
	const char *serverip = "127.0.0.1";

	if (argc < 3) {
        printf("You must input ip and port!\n");
        return -1;
	}

    serverip = argv[1];
	port = atoi(argv[2]);

    rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (rc != 0) {
        wprintf(L"WSAStartup failed: %d\n", rc);
        return 1;
    }

    sock_fd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sock_fd == INVALID_SOCKET){
        printf("Create socket failed! errno(%d)\n", WSAGetLastError());
        return -1;
    }

    memset(&serveraddr, 0, sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    serveraddr.sin_addr.s_addr = inet_addr(serverip);

    rc = connect(sock_fd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (rc == SOCKET_ERROR){
        printf("Connect server failed! errno(%d)\n", WSAGetLastError());
        return -1;
    }

    while (1) {
        rc = send(sock_fd, send_msg, send_msg_len, 0);
        if (rc == SOCKET_ERROR) {
            printf("Send ping request failed! errno(%d)\n", WSAGetLastError());
            break;
        }

        printf("%ld Send %s to server!\n", time(NULL), send_msg);

        rc = recv(sock_fd, recv_buffer, 7, 0);
        if (rc == 0) {
            printf("The connection is closed by peer!\n");
            break;
        }

        if (rc == SOCKET_ERROR) {
            printf("Receive message failed!errno(%d)\n", WSAGetLastError());
            break;
        }

        printf("%ld Recv %s from the server\n", time(NULL), recv_buffer);

        // Sleep 3 seconds
        Sleep(3000);
    }

    closesocket(sock_fd);

    return 0;
}

