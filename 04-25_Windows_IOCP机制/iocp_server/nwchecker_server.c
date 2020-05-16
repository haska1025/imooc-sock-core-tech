#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "nwcs_iocp.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

struct nwcs_iocp iocp;

int main(int argc, char *argv[])
{
    unsigned short port = 800;
    int rc = 0;
    WSADATA wsaData = { 0 };

    if (argc > 1) {
        port = atoi(argv[1]);
    }

    // Initialize winsock
    rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (rc != 0) {
        wprintf(L"WSAStartup failed: %d\n", rc);
        return -1;;
    }

    rc = nwcs_iocp_open(&iocp, port);
    if (rc != 0) {
        goto err;
    }

    nwcs_run_loop(&iocp);

    return 0;
err:
    nwcs_iocp_close(&iocp);
    WSACleanup();

    return -1;
}