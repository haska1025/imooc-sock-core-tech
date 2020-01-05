#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>

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

void gethostbyaddr_case(const char *ip)
{
    struct hostent *host = NULL;
    struct in_addr addr;

    inet_aton(ip, &addr);

    host = gethostbyaddr(&addr, sizeof(addr), AF_INET);
    if (!host){
        printf("gethostbyaddr(%s) failed! errno(%d) errorstr(%s)\n",
                ip, errno, hstrerror(errno));
        return;
    }

    printf("ip(%s) domainname(%s)\n", ip, host->h_name);
}

void getaddrinfo_case(const char *hostname, const char *service)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s;


    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
    hints.ai_flags = 0;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    s = getaddrinfo(hostname, service, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        return;
    }


    for (rp = result; rp != NULL; rp = rp->ai_next) {
        char buf[128] = {0};
        if (rp->ai_family == AF_INET){
            inet_ntop(rp->ai_family, &((struct sockaddr_in*)rp->ai_addr)->sin_addr, buf, 127);
        }else if (rp->ai_family == AF_INET6){
            inet_ntop(rp->ai_family, &((struct sockaddr_in6*)rp->ai_addr)->sin6_addr, buf, 127);
        }

        printf("hostname(%s) service(%s) family(%d) socktype(%d) protocol(%d) addr(%s)\n",
                hostname,
                service,
                rp->ai_family,
                rp->ai_socktype,
                rp->ai_protocol,
                buf);
    }

    freeaddrinfo(result);           /* No longer needed */
}

int main(int argc, char *argv[])
{
    /*
    gethostbyname_case("www.baidu.com");
    gethostbyname_case("www.google.com");

    gethostbyaddr_case("114.114.114.114");
    gethostbyaddr_case("4.4.4.4");
    gethostbyaddr_case("8.8.8.8");
*/
    getaddrinfo_case("www.imooc.com", NULL);

 //   getaddrinfo_case(NULL, "80");

    return 0;
}

