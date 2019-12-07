#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

#include "nwchecker.h"

/************** Begin of  util funciton *******************/

char * alloc_buffer(int message_size)
{
    int alloc_size = message_size <= 0?4:message_size;
    char *buff = malloc(alloc_size);
    if (alloc_size <= 5){
        memcpy(buff, "ping", alloc_size);
        buff[alloc_size - 1] = '\0';
    }

    return buff;
}
void free_buffer(char *buff)
{
    if (buff != NULL){
        free(buff);
    }
}

/******************* End of util function******************/


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
    fprintf(stderr, "  --interval -i         The send or recv interval.\n");
    fprintf(stderr, "  --send-pkgs -e        The send packages every interval.\n");
    fprintf(stderr, "  --message-size -g     The maximum package size.\n");
    fprintf(stderr, "  --echo-mode -m        The echo mode for server.1:immediate, 2: random time, 3: no response.\n");
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
            {"no-close", no_argument,      0, 'n'},
            {"port",  required_argument,   0, 'p'},
            {"address", required_argument, 0, 'a'},
            {"count", required_argument,   0, 'u'},
            {"interval", required_argument,0, 'i'},
            {"send-pkgs", required_argument,0, 'e'},
            {"message-size", required_argument,0, 'g'},
            {"echo-mode", required_argument,0, 'm'},
            {0,         0,                 0,  0 }
        };

        c = getopt_long(argc, argv, "hcsnp:a:u:i:e:g:m:", long_options, &option_index);
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
                na.port = atoi(optarg);
                break;
            case 'a':
                na.ip = optarg;
                break;
            case 'u':
                na.count = atoi(optarg);
                break;
            case 'n':
                na.no_close = 1;
                break;
            case 'i':
                na.interval = atoi(optarg);
                break;
            case 'e':
                na.sent_pkgs = atoi(optarg);
                break;
            case 'g':
                na.message_size = atoi(optarg);
                break;
            case 'm':
                na.echo_mode = atoi(optarg);
                break;
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

