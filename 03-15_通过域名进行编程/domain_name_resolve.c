#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>

void gethostbyname_case(const char *hostname)
{
    struct hostent *host = NULL;

    host = gethostbyname(hostname);
    if (!host){
        printf("gethostbyname(%s) failed! errno(%d) errorstr(%s)\n",
                hostname, errno, hstrerror(errno));
        return;
    }

    printf("h_name(%s) h_addrtype(%d) h_length(%d)\n",
            host->h_name, // CNAME
            host->h_addrtype, // AF_INET 
            host->h_length);  // 4 bytes

    char **ptr = host->h_aliases;
    for (; *ptr != NULL;){
        printf("aliases(%s)\n", *ptr++);
    }

    ptr = host->h_addr_list;
    for (; *ptr != NULL;){
        char *addr = *ptr++;
        printf("addr(%s)\n", inet_ntoa(*((struct in_addr*)addr)));
    }
}


int main(int argc, char *argv[])
{
    gethostbyname_case("www.baidu.com");

    return 0;
}

