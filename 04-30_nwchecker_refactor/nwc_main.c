#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "nwc_configuration.h"
#include "nwc.h"

int nwc_client_main(struct nwc_configuration *cfg);
int nwc_server_main(struct nwc_configuration *cfg);

int main(int argc, char *argv[])
{
    int rc = 0;
    struct nwc_configuration cfg;

    if (argc < 2){
        printf("You must select the app mode!\n");
        return -1;
    }

    memset(&cfg, 0, sizeof(struct nwc_configuration));

    rc = nwc_configuration_parse(&cfg, argc, argv);
    if (rc != 0){
        printf("Parse configure failed!\n");
        return -1;
    }

    /*
     *         if (daemon(0, 0) == -1){
     *                     printf("Create daemon failed.errno(%d)\n", errno);
     *                                 exit(-1);
     *                                         }
     */

    signal(SIGPIPE, SIG_IGN);

    if (cfg.mode == NWC_CLIENT){
        nwc_client_main(&cfg);
    }else if(cfg.mode == NWC_SERVER){
        rc = nwc_server_main(&cfg);
        if (rc != 0){
            printf("Start server failed! error(%d)\n", rc);
            return rc;
        }
    }

    return 0;
}

