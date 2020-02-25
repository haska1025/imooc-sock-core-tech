#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[])
{
    int rc = 0;
    SOCKET sock_fd = -1;
    struct sockaddr_in server_addr;

    WSADATA wsaData = { 0 };
    WSAEVENT events[WSA_MAXIMUM_WAIT_EVENTS] = { 0 }; // 事件对象数组
    SOCKET sockfds[WSA_MAXIMUM_WAIT_EVENTS] = { -1 };  // 事件对象数组对应的SOCKET句柄
    int ioevents[WSA_MAXIMUM_WAIT_EVENTS] = { 0 }; // User events
    int nr_events = 0; // 事件对象数组的数量 
    unsigned short port = 800;

    if (argc > 1) {
        port = atoi(argv[1]);
    }

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

    // 设置为非阻塞
    int flags = 1;
    ioctlsocket(sock_fd, FIONBIO, (u_long*)&flags);

    // 为服务器设置 SO_REUSEADDR 选项
    int enable = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) == SOCKET_ERROR){
        printf("setsockopt(SO_REUSEADDR) failed errno(%d)", WSAGetLastError());
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    rc = bind(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (rc == SOCKET_ERROR){
        printf("Bind server failed! errno(%d)\n", WSAGetLastError());
        return -1;
    }

    rc = listen(sock_fd, 3);
    if (rc == SOCKET_ERROR){
        printf("Server listen failed! errno(%d)\n", WSAGetLastError());
        return -1;
    }

    WSAEVENT listen_event = WSACreateEvent();
    WSAEventSelect(sock_fd, listen_event, FD_ACCEPT | FD_CLOSE);
    events[nr_events] = listen_event;
    sockfds[nr_events] = sock_fd;
    nr_events++;

    while (1) {
        printf("Enter event loop...\n");

        int lidx = 0, nidx = -1;
        for (; lidx < nr_events;) {
            if (nidx == -1) {
                if (sockfds[lidx] == -1) {
                    nidx = lidx;
                }
                else {
                    lidx++;
                }
            }
            else {
                for (; nidx < nr_events; ++nidx) {
                    if (sockfds[nidx] == -1) {
                        WSACloseEvent(events[nidx]);
                    }
                    else {
                        break;
                    }
                }
                if (nidx == nr_events)
                    break;

                events[lidx] = events[nidx];
                sockfds[lidx] = sockfds[nidx];
                ioevents[lidx] = ioevents[nidx];
                lidx++;
                nidx++;
            }
        }
        nr_events = lidx;

        int nIndex = WSAWaitForMultipleEvents(nr_events, events, FALSE, WSA_INFINITE, FALSE);
        if (nIndex == WSA_WAIT_FAILED) {
            printf("Wait for events failed.errno(%d)\n", WSAGetLastError());
            break;
        }
        nIndex = nIndex - WSA_WAIT_EVENT_0;

        int tmp_nr_events = nr_events;
        for (int n = nIndex; n < tmp_nr_events; n++) {
            nIndex = WSAWaitForMultipleEvents(1, &events[n], TRUE, 0, FALSE);
            if (WSA_WAIT_TIMEOUT == nIndex || WSA_WAIT_FAILED == nIndex) {
                continue;
            }
            WSANETWORKEVENTS event;
            SOCKET fd = sockfds[n];
            WSAEnumNetworkEvents(fd, events[n], &event);
            if (event.lNetworkEvents & FD_ACCEPT) {
                if (event.iErrorCode[FD_ACCEPT_BIT] == 0) {
                    if (nr_events >= WSA_MAXIMUM_WAIT_EVENTS) {
                        printf("The connection great than the maximum\n");
                        continue;
                    }
                    struct sockaddr_in addr;
                    memset(&addr, 0, sizeof(addr));
                    int len = sizeof(struct sockaddr_in);
                    SOCKET newfd = accept(fd, (struct sockaddr*)&addr, &len);
                    if (newfd != INVALID_SOCKET) {
                        char ipstr[32] = { 0 };
                        inet_ntop(AF_INET, &addr.sin_addr, ipstr, 31);
                        printf("Accept new connection(%s:%d)\n",
                            ipstr,
                            ntohs(addr.sin_port));
                        WSAEVENT eventNew = WSACreateEvent();
                        WSAEventSelect(newfd, eventNew, FD_READ | FD_CLOSE);
                        events[nr_events] = eventNew;
                        sockfds[nr_events] = newfd;
                        ioevents[nr_events] = 1;
                        nr_events++;
                    }
                }

                continue;// next fd
            }

            if (event.lNetworkEvents & FD_READ) {
                if (event.iErrorCode[FD_READ_BIT] == 0) {
                    char recvbuf[8] = { 0 };
                    rc = recv(fd, recvbuf, 7, 0);
                    if (rc == 0) {
                        printf("The connection is closed by peer!\n");
                        closesocket(fd);
                        sockfds[n] = -1;
                        break;
                    }

                    if (rc == SOCKET_ERROR) {
                        printf("Receive message failed!errno(%d)\n", WSAGetLastError());
                        if (errno == EWOULDBLOCK) {
                            ;// do nothing
                        }
                        else {
                            closesocket(fd);
                            sockfds[n] = -1;
                            break;
                        }
                    }
                }
            }

            if (sockfds[n] != -1 && (!(ioevents[n] & 2) || event.lNetworkEvents & FD_WRITE)) {
                // 如果设置了写事件，取消设置的写事件
                if (ioevents[n] & 2) {
                    ioevents[n] &= 1;
                    WSAEventSelect(fd, events[n], FD_READ | FD_CLOSE);
                }

                const char *pong = "pong";
                size_t pong_len = strlen(pong);
                rc = send(fd, pong, pong_len, 0);
                if (rc == SOCKET_ERROR) {
                    printf("Send ping reponse failed! errno(%d)\n", WSAGetLastError());
                    if (errno == EWOULDBLOCK) {
                        ioevents[n] |= 2;
                        WSAEventSelect(fd, events[n], FD_READ | FD_CLOSE | FD_WRITE);
                    }
                    else {
                        closesocket(fd);
                        sockfds[n] = -1;
                        break;
                    }
                }
                printf("%ld Send %s to client !\n", time(NULL), pong);
            }

            if (sockfds[n] != -1 && event.lNetworkEvents & FD_CLOSE) {
                closesocket(fd);
                sockfds[n] = -1;
            }
        }
    }

    closesocket(sock_fd);

    return 0;
}

