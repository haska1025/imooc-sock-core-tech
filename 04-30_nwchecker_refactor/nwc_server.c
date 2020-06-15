#include <stdlib.h>
#include <stdio.h>

#include "nwc.h"
#include "nwc_connection.h"

#define DEFAULT_PORT 9820

struct nwc_io_operation *listen_op = NULL;
nwc_handle_t listen_handle = INVALID_HANDLE;
struct list_head conn_hdr;

static struct nwc_io_operation *nwc_server_accept(void *userdata, nwc_handle_t handle)
{
    struct nwc_configuration *cfg = (struct nwc_configuration*)userdata;

    if(cfg->prototype == NWC_PROTO_UDP){
        nwc_connection_udp_accept(handle, cfg);
        return NULL;
    }else{
        struct nwc_connection *conn = malloc(sizeof(struct nwc_connection));

        nwc_connection_init(conn, cfg);
        conn->handle = handle;

        // Save the connection to connection list
        list_add_tail(&conn->conn_entry, &conn_hdr);

        return (struct nwc_io_operation*)conn;
    }
}

static int nwc_server_start(struct nwc_configuration *cfg, nwc_handle_t looper)
{
    INIT_LIST_HEAD(&conn_hdr);

    listen_op = malloc(sizeof(struct nwc_io_operation));

    nwc_io_operation_init(listen_op);
    // Let the configuration as the userdata
    listen_op->userdata = cfg;
    listen_op->on_accept = nwc_server_accept;

    listen_handle = nwc_io_create(looper, listen_op, cfg->prototype);

    return nwc_io_listen(listen_handle, cfg->ip, cfg->port);
}

static int nwc_server_stop()
{
    nwc_io_close(listen_handle);
    free(listen_op);
    listen_op = NULL;

    return 0;
}

int nwc_server_main(struct nwc_configuration *cfg)
{
    int rc = 0;
    nwc_handle_t looper = nwc_looper_create();

    rc = nwc_looper_start(looper);
    if (rc != 0)
        return rc;

    if (cfg->port == 0){
        cfg->port = DEFAULT_PORT;
    }
    if (cfg->prototype == 0){
        cfg->prototype = NWC_PROTO_TCP;
    }

    if (cfg->prototype == NWC_PROTO_TCP
            || cfg->prototype == NWC_PROTO_UDP){
        rc = nwc_server_start(cfg, looper);
        if (rc != 0)
            return rc;
    }else{
        printf("Start server no prototype(%d)\n", cfg->prototype);
        return -1;
    }

    nwc_looper_run(looper);

    nwc_server_stop();

    return 0;
}

