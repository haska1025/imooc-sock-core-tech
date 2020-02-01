#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include "nwchecker.h"

/************** Begin of  util funciton *******************/

char * alloc_buffer(int alloc_size)
{
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
    fprintf(stderr, "  --local-address -l    The client local address.\n");
    fprintf(stderr, "  --count -o            The number of the test.\n");
    fprintf(stderr, "  --no-close -n         Don't close the socket.\n");
    fprintf(stderr, "  --interval -i         The send or recv interval.\n");
    fprintf(stderr, "  --send-pkgs -e        The send packages every interval.\n");
    fprintf(stderr, "  --message-size -g     The maximum package size.\n");
    fprintf(stderr, "  --echo-mode -m        The echo mode for server.0:immediate, 1: random time, 2: no response.\n");
    fprintf(stderr, "  --udp -u              The UDP Socket.\n");
    fprintf(stderr, "  --tcp -t              The TCP Socket.\n");
    fprintf(stderr, "  --unix-stream         The UNIX Stream Socket.\n");
    fprintf(stderr, "  --unix-dgram          The UNIX Dgram Socket.\n");
    fprintf(stderr, "  --connect             The UDP client with connect socket.\n");

    exit(0);
}

int main(int argc, char *argv[])
{
    int c;
    int mode=0;
    int trans_type = 0;// 1:tcp, 2:udp, 3:unix-stream, 4:unix-dgram

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
            {"local-address", required_argument, 0, 'l'},
            {"count", required_argument,   0, 'o'},
            {"interval", required_argument,0, 'i'},
            {"send-pkgs", required_argument,0, 'e'},
            {"message-size", required_argument,0, 'g'},
            {"echo-mode", required_argument,0, 'm'},
            {"tcp", no_argument,            0, 't'},
            {"udp", no_argument,            0, 'u'},
            {"connect", no_argument,        0,  1 },
            {"unix-stream", no_argument,    0,  2 },
            {"unix-dgram", no_argument,     0,  3 },
            {0,         0,                  0,  0 }
        };

        c = getopt_long(argc, argv, "uthcsnp:a:l:o:u:i:e:g:m:", long_options, &option_index);
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
            case 'l':
                na.localaddr = optarg;
                break;
            case 'o':
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
            case 't':
                trans_type = 1;
                break;
            case 'u':
                trans_type = 2;
                break;
            case 1:
                na.is_udp_connect = 1;
                break;
            case 2:
                trans_type = 3;
                break;
            case 3:
                trans_type = 4;
                break;
            default:
                usage();
        }
    }

    if (mode == 1){
        // nwc client 
        if (trans_type == 1){
            nwc_client(&na);
        }else if (trans_type == 2){
            nwc_udp_client(&na);
        }else if(trans_type == 3){
            unix_stream_client(&na);
        }else if(trans_type == 4){
            unix_dgram_client(&na);
        }
    }else if (mode == 2){
        /*
        if (daemon(0, 0) == -1){
            printf("Create daemon failed.errno(%d)\n", errno);
            exit(-1);
        }
        */

        signal(SIGPIPE, SIG_IGN);
        // nwc server 
        if (trans_type == 1){
            nwc_server(&na);
        }else if (trans_type == 2){
            nwc_udp_server(&na);
        }else if (trans_type == 3){
            unix_stream_server(&na);
        }else if(trans_type == 4){
            unix_dgram_server(&na);
        }
    }

    return 0;
}

