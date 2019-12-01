#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>

#include "nwchecker.h"

void usage()
{
    fprintf(stderr, "Usage: nwchecker [option]\n");
    fprintf(stderr, "  --help -h             help information\n");
    fprintf(stderr, "  --client -c           The client mode.\n");
    fprintf(stderr, "  --server -s           The server mode.\n");
    fprintf(stderr, "  --port -p             The server port.\n");
    fprintf(stderr, "  --address -a          The server address.\n");
    fprintf(stderr, "  --count -u            The number of the test.\n");
    fprintf(stderr, "  --no-close -n         Don't close the socket.\n");
    exit(0);
}

int main(int argc, char *argv[])
{
    int c;
    int mode=0;

    struct nwc_args na;

    memset(&na, 0, sizeof(na));

    if (argc < 2){
        usage();
        return 0;
    }

    while (1) {
        int option_index = 0;
        static struct option long_options[] = {
            {"help",    no_argument,       0, 'h'},
            {"client",  no_argument,       0, 'c'},
            {"server",  no_argument,       0, 's'},
            {"port",  no_argument,         0, 'p'},
            {"address",no_argument,        0, 'a'},
            {"count", no_argument,         0, 'u'},
            {"no-close", no_argument,      0, 'n'},
            {0,         0,                 0,  0 }
        };

        c = getopt_long(argc, argv, "hcspaun", long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 'h':
                usage();
                break;
            case 'c':
                mode = 1;
                break;
            case 's':
                mode =2;
                break;
            case 'p':
                na->port = atoi(optarg);
                break;
            case 'a':
                na->ip = optarg;
                break;
            case 'u':
                na->count = atoi(optarg);
                break;
            case 'n':
                na->no_close = 1;
            default:
                usage();
        }
    }

    if (mode == 1){
        // nwc client 
        nwc_client(&na);
    }else if (mode == 2){
        // nwc server 
        nwc_server(&na);
    }

    return 0;
}

