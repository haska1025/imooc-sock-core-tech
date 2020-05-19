#include "nwcs_iocp.h"
#include <stdio.h>

int nwcs_iocp_open(struct nwcs_iocp *iocp, unsigned short port)
{
    int rc = 0;
    SOCKET sock_fd = -1;
    struct sockaddr_in server_addr;
    GUID GuidAcceptEx = WSAID_ACCEPTEX;
    DWORD dwBytes;

    // Create io completion port
    iocp->completion_port = CreateIoCompletionPort(INVALID_HANDLE_VALUE,
        NULL,
        0,
        0);
    if (iocp->completion_port == NULL) {
        printf("Create io completeion failed error(%d)", WSAGetLastError());
        return -1;
    }

    INIT_NWC(&iocp->client_conn_hdr);
    INIT_NWC(&iocp->delete_client_conn_hdr);

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == INVALID_SOCKET) {
        printf("Create socket failed! errno(%d)\n", WSAGetLastError());
        return -1;
    }

    // 设置为非阻塞
    int flags = 1;
    ioctlsocket(sock_fd, FIONBIO, (u_long*)&flags);

    // 为服务器设置 SO_REUSEADDR 选项
    const char enable = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) == SOCKET_ERROR) {
        printf("setsockopt(SO_REUSEADDR) failed errno(%d)", WSAGetLastError());
        closesocket(sock_fd);
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    rc = bind(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (rc == SOCKET_ERROR) {
        printf("Bind server failed! errno(%d)\n", WSAGetLastError());
        closesocket(sock_fd);
        return -1;
    }

    rc = listen(sock_fd, 3);
    if (rc == SOCKET_ERROR) {
        printf("Server listen failed! errno(%d)\n", WSAGetLastError());
        closesocket(sock_fd);
        return -1;
    }

    INIT_NWC(&iocp->accept_conn);
    iocp->accept_conn.socket = sock_fd;

    HANDLE h = CreateIoCompletionPort((HANDLE)sock_fd, iocp->completion_port, (ULONG_PTR)&iocp->accept_conn, 0);
    if (h != iocp->completion_port) {
        printf("Associate the socket with io completion port failed");
        return -1;
    }

    rc = WSAIoctl(sock_fd,
        SIO_GET_EXTENSION_FUNCTION_POINTER,
        &GuidAcceptEx,sizeof(GuidAcceptEx),
        &iocp->accept_conn.lpfnAcceptEx, sizeof(iocp->accept_conn.lpfnAcceptEx),
        &dwBytes, NULL, NULL);
    if (rc == SOCKET_ERROR) {
        wprintf(L"WSAIoctl failed with error: %u\n", WSAGetLastError());
        return -1;
    }

    nwcs_accept_request(iocp);
    iocp->is_exit = FALSE;

    return 0;
}

int nwcs_accept_request(struct nwcs_iocp *iocp)
{
    SOCKET newsock = INVALID_SOCKET;
    BOOL bRetVal = FALSE;
    DWORD dwBytes;
    // Create an accepting socket
    newsock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (newsock == INVALID_SOCKET) {
        wprintf(L"Create accept socket failed with error: %u\n", WSAGetLastError());
        return -1;
    }

    // Empty our overlapped structure and accept connections.
    struct nwc_io_op *ioop = alloc_nwc_io_op(OP_ACCEPT);

    bRetVal = iocp->accept_conn.lpfnAcceptEx(iocp->accept_conn.socket, newsock,
        iocp->accept_conn.recv_buff,
        0,
        sizeof(struct sockaddr_in) + 16,
        sizeof(struct sockaddr_in) + 16,
        &dwBytes, &ioop->olapped);
    if (bRetVal == FALSE && WSAGetLastError() != ERROR_IO_PENDING) {
        wprintf(L"AcceptEx failed with error: %u\n", WSAGetLastError());
        closesocket(newsock);
        free_nwc_io_op(ioop);
        return -1;
    }

    ioop->socket = newsock;

    return 0;
}

int nwcs_accept_new_sock(struct nwcs_iocp *iocp, struct nwc_connection *nwc)
{
    int iResult = 0;

    // 设置为非阻塞
    int flags = 1;
    ioctlsocket(nwc->socket, FIONBIO, (u_long*)&flags);

    // Set SO_UPDATE_ACCEPT_CONTEXT
    iResult = setsockopt(nwc->socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
        (char *)&iocp->accept_conn.socket, sizeof(iocp->accept_conn.socket));
    if (iResult != 0) {
        wprintf(L"setsockopt set newsock with SO_UPDATE_ACCEPT_CONTEXT failed with error: %u\n",
            GetLastError());
        return -1;
    }

    // Associate the accept socket with the completion port
    HANDLE h = CreateIoCompletionPort((HANDLE)nwc->socket, iocp->completion_port, (ULONG_PTR)nwc, 0);
    // h should be iocp->completion_port if this succeeds
    if (h != iocp->completion_port) {
        wprintf(L"CreateIoCompletionPort associate failed with error: %u\n",
            GetLastError());
        return -1;
    }
    return 0;
}

int nwcs_read_request(struct nwc_connection *nwc)
{
    int rc = 0;
    DWORD flags = 0;
    DWORD rbytes = 0;
    struct nwc_io_op *ioop = alloc_nwc_io_op(OP_READ);

    rc = WSARecv(nwc->socket,
        &nwc->wsabuf,
        1,
        &rbytes,
        &flags,
        &ioop->olapped,
        NULL);

    if (rc ==  SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING) {
        wprintf(L"WSARecv failed with error: %u\n",
            WSAGetLastError());
        free_nwc_io_op(ioop);
        ioop = NULL;

        return -1;
    }

    // socket recv eof
    if (rc == 0 && rbytes == 0) {
        wprintf(L"WSARecv connection closed by peer.\n");
        free_nwc_io_op(ioop);
        ioop = NULL;

        return -1;
    }

    nwc->pending_ops++;

    return 0;
}

int nwcs_write_request(struct nwc_connection *nwc, const char *buff, unsigned int buflen)
{
    int rc = 0;
    DWORD sbytes = 0;
    WSABUF sbuf;
    struct nwc_io_op *ioop = alloc_nwc_io_op(OP_WRITE);

    sbuf.buf = (char*)buff;
    sbuf.len = buflen;

    rc = WSASend(nwc->socket,
        (WSABUF*)&sbuf,
        1,
        &sbytes,
        0,
        &ioop->olapped,
        NULL);

    if (rc == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING) {
        wprintf(L"WSASend failed with error(%d).\n", WSAGetLastError());
        free_nwc_io_op(ioop);
        ioop = NULL;
        return -1;
    }

    nwc->pending_ops++;

    return 0;
}
void nwcs_run_loop(struct nwcs_iocp *iocp)
{
    printf("Enter run loop....\n");

    while (!iocp->is_exit) {
        DWORD rbytes = 0;
        struct nwc_connection *nwc = NULL;
        LPOVERLAPPED overlapped = NULL;
        BOOL iRet = FALSE;
        struct nwc_io_op *ioop = NULL;

        iRet = GetQueuedCompletionStatus(
            iocp->completion_port,
            &rbytes,
            (PULONG_PTR)&nwc,
            &overlapped,
            INFINITE);

        if (!iRet) {
            printf("GetQueuedCompletionStatus failed error(%d)\n", GetLastError());
            if (overlapped) {
                ioop = CONTAINING_RECORD(overlapped, struct nwc_io_op, olapped);
                free_nwc_io_op(ioop);
                ioop = NULL;
            }
            if (nwc) {
                remove_nwc(nwc);
                add_nwc_tail(&iocp->delete_client_conn_hdr, nwc);
            }
            continue;
        }

        if (overlapped) {
            ioop = CONTAINING_RECORD(overlapped, struct nwc_io_op, olapped);
            if (nwc) {
                nwc->rbytes = rbytes;
                nwcs_iocp_process_message(iocp, nwc, ioop);
            }

            // Free the oper memory
            free_nwc_io_op(ioop);
            ioop = NULL;
            nwc->pending_ops--;
        }

        // Free
        struct nwc_connection *d_nwc = iocp->delete_client_conn_hdr.next;
        for (; d_nwc != &iocp->delete_client_conn_hdr;) {
            struct nwc_connection *next = d_nwc->next;
            if (d_nwc->pending_ops <= 0) {
                d_nwc->prev->next = next;
                next->prev = d_nwc->prev;
                printf("Free unused nwc.\n");
                free_nwc_conn(d_nwc);
            }

            d_nwc = next;
        }
    }
}

void nwcs_iocp_process_message(struct nwcs_iocp *iocp, struct nwc_connection *nwc, struct nwc_io_op *ioop)
{
    int rc = 0;

    if (ioop->op == OP_ACCEPT) {
        // Post next accept event
        nwcs_accept_request(iocp);

        // Create new connection
        struct nwc_connection *newnwc = alloc_nwc_conn_arg0();
        newnwc->socket = ioop->socket;
        rc = nwcs_accept_new_sock(iocp, newnwc);
        if (rc != 0) {
            free_nwc_conn(newnwc);
            newnwc = NULL;
            return;
        }
        rc = nwcs_read_request(newnwc);
        if (rc != 0) {
            free_nwc_conn(newnwc);
            newnwc = NULL;
            return;
        }
        add_nwc_tail(&iocp->client_conn_hdr, newnwc);
        printf("WSARecv new socket(%d) rc(%d)\n", ioop->socket, rc);
    }
    else if (ioop->op == OP_READ) {
        nwc->recv_buff[nwc->rbytes] = '\0';
        printf("Recv client message(%s)\n", nwc->recv_buff);

        const char *pong = "pong";
        size_t pong_len = strlen(pong);

        rc = nwcs_write_request(nwc, pong, pong_len);
        if (rc != 0) {
            remove_nwc(nwc);
            add_nwc_tail(&iocp->delete_client_conn_hdr, nwc);
            return;
        }
        rc = nwcs_read_request(nwc);
        if (rc != 0) {
            remove_nwc(nwc);
            add_nwc_tail(&iocp->delete_client_conn_hdr, nwc);
            return;
        }
    }
    else if (ioop->op == OP_WRITE) {
        printf("Send client message over\n");
    }
}

int nwcs_iocp_close(struct nwcs_iocp *iocp)
{
    CloseHandle(iocp->completion_port);
    iocp->completion_port = INVALID_HANDLE_VALUE;

    closesocket(iocp->accept_conn.socket);
    iocp->accept_conn.socket = INVALID_SOCKET;

    destroy_all_nwc(&iocp->client_conn_hdr);

    return 0;
}
