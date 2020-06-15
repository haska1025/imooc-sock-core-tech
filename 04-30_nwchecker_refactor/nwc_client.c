#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include "nwc.h"
#include "nwc_connection.h"
#include "nwc_configuration.h"

static struct nwc_connection *conn = NULL;

static int nwc_client_start(struct nwc_configuration *cfg, nwc_handle_t looper)
{
    int rc = 0;

    conn = malloc(sizeof(struct nwc_connection)); 
    nwc_connection_init(conn, cfg);

    conn->handle = nwc_io_create(looper, &conn->op, cfg->prototype);
    rc = nwc_io_connect(conn->handle, cfg->ip, cfg->port);
    printf("nwc_client_start connect to server ip(%s) port(%d) prototype(%d) rc(%d)\n",
            cfg->ip, cfg->port, cfg->prototype, rc);
    if (rc == 0){
        nwc_connection_send(conn);
    }else if (rc == -EINPROGRESS){
        ;//do nothing
    }else{
        return rc;
    }

    return 0;
}

int nwc_client_main(struct nwc_configuration *cfg)
{
    int rc = 0;

    if (!cfg->ip || !cfg->port) {
        printf("Please input server ip and port!\n");
        return -1;
    }

    nwc_handle_t looper = nwc_looper_create();

    rc = nwc_looper_start(looper);
    if (rc != 0)
        return rc;

    if (cfg->prototype == 0)
        cfg->prototype = NWC_PROTO_TCP;

    if (cfg->prototype == NWC_PROTO_TCP 
            || cfg->prototype == NWC_PROTO_UDP){
        rc = nwc_client_start(cfg, looper);
        if (rc != 0){
            printf("Start client failed prototype(%d) rc(%d)\n", cfg->prototype, rc);
            return rc;
        }
    }else{
        printf("Start client no prototype(%d)\n", cfg->prototype);
        return -1;
    }

    nwc_looper_run(looper);

    nwc_looper_stop(looper);
    nwc_looper_destroy(looper);

    return 0;
}

