#ifndef _NWC_IOCPS_H_
#define _NWC_IOCPS_H_

#include "nwc_connection.h"

#ifdef __cplusplus
extern "C" {
#endif

    // An IOCP Instance
    struct nwcs_iocp
    {
        HANDLE completion_port;
        struct nwc_connection accept_conn;
        struct nwc_connection client_conn_hdr;
        struct nwc_connection delete_client_conn_hdr;
        BOOL is_exit;
    };

    int nwcs_iocp_open(struct nwcs_iocp *iocp, unsigned short port);
    int nwcs_iocp_close(struct nwcs_iocp *iocp);
    int nwcs_accept_new_sock(struct nwcs_iocp *iocp, struct nwc_connection *nwc);
    int nwcs_accept_request(struct nwcs_iocp *iocp);
    void nwcs_run_loop(struct nwcs_iocp *iocp);
    void nwcs_iocp_process_message(struct nwcs_iocp *iocp, struct nwc_connection *nwc, struct nwc_io_op *ioop);

    int nwcs_read_request(struct nwc_connection *nwc);
    int nwcs_write_request(struct nwc_connection *nwc, const char *buff, unsigned int bulen);

#ifdef __cplusplus
}
#endif

#endif // !_NWC_IOCPS_H_
