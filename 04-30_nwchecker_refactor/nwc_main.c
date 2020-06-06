#include <stdio.h>
#include "nwc.h"

int main(int argc, char *argv[])
{
    int port = 9820; 
    nwc_handle_t tcp_listen = INVALID_HANDLE;
    nwc_handle_t looper = nwc_looper_create();
    nwc_looper_start(looper);

    tcp_listen = nwc_tcp_listen_start(looper, NULL, port);
    
    nwc_looper_run(looper);
   
    nwc_looper_destroy(looper);
    return 0;
}

