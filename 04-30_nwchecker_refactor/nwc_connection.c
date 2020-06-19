#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "nwc_connection.h"

extern struct list_head conn_hdr;

static int nwc_connection_stat(void *ud)
{
    struct nwc_connection *nwc = (struct nwc_connection*)ud;
    nwc_qos_stat(&nwc->qos);
}

struct nwc_connection *alloc_nwc_conn_arg0()
{
    struct nwc_connection *nwc = (struct nwc_connection*)malloc(sizeof(struct nwc_connection));

    INIT_LIST_HEAD(&nwc->conn_entry);
    nwc->rbytes = 0;

    return nwc;
}

void free_nwc_conn(struct nwc_connection *nwc)
{
    if (nwc){
        free(nwc);
        nwc = NULL;
    }
}

void nwc_connection_init(struct nwc_connection *nwc, struct nwc_configuration *cfg)
{
    uint32_t bd = 0;
    
    nwc_io_operation_init(&nwc->op);
    nwc_qos_init(&nwc->qos);
    nwc->op.on_connect = nwc_connection_on_connect;
    nwc->op.on_recv = nwc_connection_on_recv;
    nwc->op.on_send = nwc_connection_on_send;
    nwc->op.userdata = nwc;

    memset(&nwc->cliaddr, 0, sizeof(nwc->cliaddr));
    nwc->cliaddrlen = sizeof(nwc->cliaddr);
    nwc->handle = INVALID_HANDLE;
    nwc->rbytes = 0;
    nwc->cfg = cfg;
    nwc->stat_wk_handle = INVALID_HANDLE;
    nwc->send_wk_handle = INVALID_HANDLE;

    bd = cfg->bandwidth;

    // Just client send data
    if (nwc->cfg->mode == NWC_CLIENT){ 
        if (bd == 0){
            bd = 125000;// 1000Kbps / 8 = 125KBps
        }else{
            bd >>= 3; // The unit of the user-defined bandwidth is bps
        }

        bd = (bd + 19) / 20;
        nwc->qos.send_bandwidth = bd;
    }
}

void nwc_connection_on_connect(void *userdata)
{
    struct nwc_connection *nwc = (struct nwc_connection*)userdata;
    printf("Connect successfully!\n");
    nwc_connection_send(nwc);
}
int nwc_connection_on_recv(void *userdata)
{
    int rc = 0;
    int rbytes = 0;
    uint32_t qos_hdr_len = sizeof(struct nwc_qos_hdr);
    uint8_t *pkg_ptr = NULL;
    struct nwc_qos_hdr *pkg = NULL;
    struct nwc_connection *nwc = (struct nwc_connection*)userdata;
    uint32_t recv_ts = 0;

    rc = nwc_io_recv(nwc->handle, nwc->recv_buff + nwc->rbytes, RECV_BUFFER_LEN - nwc->rbytes);
    if (rc <= 0){
        if(rc == -EAGAIN){
            return 0;
        }
        nwc_connection_close(nwc);
        printf("Connection recv meet error(%d)\n", rc);
        return rc;
    }

    recv_ts = nwc_qos_milisecs();

    rbytes = rc + nwc->rbytes;
    pkg_ptr = nwc->recv_buff;

    while (1){
        if (rbytes < qos_hdr_len)
            break;

        pkg = (struct nwc_qos_hdr*)pkg_ptr;
        if (rbytes < qos_hdr_len + pkg->length)
            break;

        // Process message
        if (nwc->cfg->mode == NWC_SERVER){
            // Server
            uint32_t nowms = 0;
            uint8_t *buffer = NULL;
            uint32_t buflen = 0;
            struct nwc_qos_hdr *ack_pkg = NULL;
            if (!nwc_io_can_send(nwc->handle))
                continue;

            ack_pkg = nwc_qos_create_ack_package(&nwc->qos, pkg->timestamp); 
            buffer = (uint8_t*)ack_pkg;
            buflen = sizeof(struct nwc_qos_hdr) + ack_pkg->length;

            nowms = nwc_qos_milisecs();
            ack_pkg->recv_delay = nowms - recv_ts;
            ack_pkg->timestamp = nowms;
            rc = nwc_io_send(nwc->handle, buffer, buflen); 
#ifdef _DEBUG
            printf("nwc_io_recv send response pkg len(%u) rc(%d)\n", buflen, rc);
#endif
            free(ack_pkg);
            ack_pkg = NULL;

            if (rc <= 0){
                if (rc != -EAGAIN){
                    nwc_connection_close(nwc);
                    printf("Connection send meet error(%d)\n", rc);
                    break;
                }
            }
        }else{
            // Client
            nwc_qos_process_package(&nwc->qos, pkg, recv_ts);
        }

        pkg_ptr = pkg_ptr + qos_hdr_len + pkg->length;
        rbytes -= qos_hdr_len + pkg->length;
    }

    if (rbytes > 0){
        memmove(nwc->recv_buff, pkg_ptr , rbytes);
        nwc->rbytes = rbytes;
    }else{
        nwc->rbytes = 0;
    }

    return 0;
}
int nwc_connection_on_send(void *userdata)
{
    int rc = 0;
    struct nwc_connection *nwc = (struct nwc_connection*)userdata;
    struct nwc_qos_hdr *pkg = NULL;
    uint8_t *buffer = NULL;
    uint32_t buflen = 0;

    // Can't send data
    if (!nwc_io_can_send(nwc->handle))
        return 0;

    if (nwc->cfg->mode == NWC_CLIENT){
        uint32_t cur_sentbytes = 0;
        while (cur_sentbytes < nwc->qos.send_bandwidth){
            pkg = nwc_qos_create_package(&nwc->qos, 1300); 
            buffer = (uint8_t*)pkg;
            buflen = sizeof(struct nwc_qos_hdr) + pkg->length;
            pkg->timestamp = nwc_qos_milisecs();

            rc = nwc_io_send(nwc->handle, buffer, buflen); 
#ifdef _DEBUG
            printf("nwc_io_send bd(%d) cur_bd(%d) pkg len(%u) rc(%d)\n",
                    nwc->qos.send_bandwidth, cur_sentbytes, buflen, rc);
#endif

            if (rc <= 0){
                if (rc != -EAGAIN){
                    nwc_connection_close(nwc);
                }
                printf("Connection send meet error(%d)\n", rc);
                break;
            }

            cur_sentbytes += rc;
        }
    }

    return rc;
}

int nwc_connection_send(struct nwc_connection *nwc)
{
    nwc->stat_wk_handle = nwc_looper_register_worker(nwc_io_get_looper(nwc->handle), nwc, nwc_connection_stat);
    nwc->send_wk_handle = nwc_looper_register_worker(nwc_io_get_looper(nwc->handle), nwc, nwc_connection_on_send);

    return 0;
}

void nwc_connection_close(struct nwc_connection *nwc)
{
    nwc_io_close(nwc->handle);
    nwc_looper_cancel_worker(nwc_io_get_looper(nwc->handle), nwc->stat_wk_handle);
    nwc_looper_cancel_worker(nwc_io_get_looper(nwc->handle), nwc->send_wk_handle);

    free_nwc_conn(nwc);
    nwc = NULL;
}

void nwc_connection_udp_accept(nwc_handle_t handle, struct nwc_configuration *cfg)
{
    int rc = 0;
    struct nwc_connection *nwc;
    uint32_t nowms = 0;
    uint8_t *buffer = NULL;
    uint32_t buflen = 0;
    struct nwc_qos_hdr *pkg = NULL, *ack_pkg = NULL;

    uint8_t recvbuff[RECV_BUFFER_LEN];
    struct sockaddr_storage cliaddr;
    socklen_t cliaddrlen = sizeof(cliaddr);
    memset(&cliaddr, 0, cliaddrlen);

    rc = nwc_io_recvfrom(handle, recvbuff, RECV_BUFFER_LEN, (struct sockaddr*)&cliaddr, &cliaddrlen); 
    if (rc <= 0){
        if (rc != -EAGAIN){
            printf("nwc udp accept failed rc(%d)\n", rc);
        }
        return;
    }

    nwc = get_nwc_connection(&conn_hdr, (struct sockaddr*)&cliaddr);
    if (!nwc){
        nwc = malloc(sizeof(struct nwc_connection));
        nwc_connection_init(nwc, cfg);
        add_nwc_tail(&conn_hdr, nwc);
        memcpy(&nwc->cliaddr, &cliaddr, cliaddrlen);
        nwc->cliaddrlen = cliaddrlen;
        printf("UDP Server create new nwc\n");
    }

    // Process message
    pkg = (struct nwc_qos_hdr*)recvbuff;
    ack_pkg = nwc_qos_create_ack_package(&nwc->qos, pkg->timestamp); 
    buffer = (uint8_t*)ack_pkg;
    buflen = sizeof(struct nwc_qos_hdr) + ack_pkg->length;

    nowms = nwc_qos_milisecs();
    ack_pkg->recv_delay = 0;
    ack_pkg->timestamp = nowms;
    rc = nwc_io_sendto(handle, buffer, buflen, (struct sockaddr*)&cliaddr, cliaddrlen); 
#ifdef _DEBUG
    printf("nwc_io_sendto send response pkg len(%u) rc(%d)\n", buflen, rc);
#endif
    free(ack_pkg);
    ack_pkg = NULL;
}

struct nwc_connection * get_nwc_connection(struct list_head *head, struct sockaddr *addr)
{
    struct list_head *pos = NULL;
    struct nwc_connection *nwc = NULL;
    struct sockaddr_in *srcaddr = NULL;
    struct sockaddr_in *dstaddr = (struct sockaddr_in*)addr;

    list_for_each(pos, head){
        nwc = list_entry(pos, struct nwc_connection, conn_entry);
        srcaddr = (struct sockaddr_in*)(&nwc->cliaddr);
        if (srcaddr->sin_port == dstaddr->sin_port
                && srcaddr->sin_addr.s_addr == dstaddr->sin_addr.s_addr)
            return nwc;
    }

    return NULL;
}

void add_nwc_tail(struct list_head *head, struct nwc_connection *nwc)
{
    list_add_tail(&nwc->conn_entry, head);
}

void remove_nwc(struct nwc_connection *nwc)
{
    list_del(&nwc->conn_entry);
}
void destroy_all_nwc(struct list_head *hdr)
{
    struct list_head *pos, *n = NULL;
    struct nwc_connection *nwc = NULL;

    list_for_each_safe(pos, n, hdr){
        struct nwc_connection *nwc = list_entry(pos, struct nwc_connection, conn_entry);
        free_nwc_conn(nwc);
        nwc = NULL;
    }
}

