#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include "nwc_types.h"
#include "nwc_configuration.h"

static void usage()
{
    fprintf(stderr, "Usage: nwchecker [option]\n");
    fprintf(stderr, "  --help -h             help information\n");
    fprintf(stderr, "  --client -c           The client mode.\n");
    fprintf(stderr, "  --server -s           The server mode.\n");
    fprintf(stderr, "  --port -p             The server port.\n");
    fprintf(stderr, "  --address -a          The server address.\n");
    fprintf(stderr, "  --count -o            The total test time, unit is seconds.\n");
    fprintf(stderr, "  --bandwidth -b        The user-defined bandwidth.\n");
    fprintf(stderr, "  --udp -u              The UDP Socket.\n");
    fprintf(stderr, "  --tcp -t              The TCP Socket.\n");

    exit(0);
}

int nwc_configuration_parse(struct nwc_configuration *config, int argc, char *argv[])
{
    while (1) {
        int c = 0;
        int option_index = 0;
        static struct option long_options[] = {
            {"help",    no_argument,       0, 'h'},
            {"client",  no_argument,       0, 'c'},
            {"server",  no_argument,       0, 's'},
            {"port",  required_argument,   0, 'p'},
            {"address", required_argument, 0, 'a'},
            {"count", required_argument,   0, 'o'},
            {"bandwidth", required_argument,0, 'b'},
            {"tcp", no_argument,            0, 't'},
            {"udp", no_argument,            0, 'u'},
            {0,         0,                  0,  0 }
        };

        c = getopt_long(argc, argv, "uthcsp:a:o:b:", long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 'h':
                usage();
                break;
            case 'c':
                config->mode = NWC_CLIENT;
                break;
            case 's':
                config->mode = NWC_SERVER;
                break;
            case 'p':
                config->port = atoi(optarg);
                break;
            case 'a':
                config->ip = optarg;
                break;
            case 'o':
                config->count_secs = atoi(optarg);
                break;
            case 'b':
                config->bandwidth = atoi(optarg);
                break;
            case 't':
                config->prototype = NWC_PROTO_TCP;
                break;
            case 'u':
                config->prototype = NWC_PROTO_UDP;
                break;
            default:
                usage();
        }
    }

    return 0;
}

