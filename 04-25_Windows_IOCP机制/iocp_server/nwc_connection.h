#ifndef _NWC_CONNECTION_H_
#define _NWC_CONNECTION_H_

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <Windows.h>

#ifdef __cplusplus
extern "C" {
#endif

enum e_oper
{
    OP_READ = 1,
    OP_ACCEPT,
    OP_WRITE
};

struct nwc_io_op
{
    OVERLAPPED olapped;
    int op;
    SOCKET socket;};

struct nwc_io_op * alloc_nwc_io_op(int op);
void free_nwc_io_op(struct nwc_io_op *op);

#define RECV_BUFFER_LEN 4096

struct nwc_connection
{
    struct nwc_connection *next;
    struct nwc_connection *prev;

    SOCKET socket;
    LPFN_ACCEPTEX lpfnAcceptEx;
    struct sockaddr_in addr;
    BYTE recv_buff[RECV_BUFFER_LEN];
    DWORD rbytes;

    WSABUF wsabuf;
    int pending_ops;
};

#define INIT_NWC(nwc)\
    do { \
        (nwc)->next = (nwc);\
        (nwc)->prev = (nwc);\
        (nwc)->socket = -1;\
        (nwc)->lpfnAcceptEx = NULL;\
        (nwc)->pending_ops = 0;\
    }while(0);

struct nwc_connection *alloc_nwc_conn_arg0();
void free_nwc_conn(struct nwc_connection *nwc);

struct nwc_connection * get_nwc_connection(struct nwc_connection *hdr, struct sockaddr *addr);

void add_nwc_tail(struct nwc_connection *hdr, struct nwc_connection *nwc);
void remove_nwc(struct nwc_connection *nwc); 
void destroy_all_nwc(struct nwc_connection *hdr);

#ifdef __cplusplus
}
#endif
#endif//_NWC_CONNECTION_H_
