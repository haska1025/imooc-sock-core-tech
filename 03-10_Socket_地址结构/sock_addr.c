#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>

void test_sock_addr_length()
{
    printf("The length of struct sockaddr_in: %d\n", sizeof(struct sockaddr_in));
    printf("The length of struct sockaddr_in6: %d\n", sizeof(struct sockaddr_in6));
    printf("The length of struct sockaddr_un: %d\n", sizeof(struct sockaddr_un));
    printf("The length of struct sockaddr: %d\n", sizeof(struct sockaddr));
    printf("The length of struct sockaddr_storage: %d\n", sizeof(struct sockaddr_storage));
}

void init_sock_addr(int af, struct sockaddr_storage *ss, unsigned short port, const char *addr)
{
    ss->ss_family = af;
    if (af == AF_INET){
        struct sockaddr_in *si = (struct sockaddr_in*)ss;
        si->sin_port = htons(port);
        si->sin_addr.s_addr = inet_addr(addr);
    }else if (af == AF_INET6){
        struct sockaddr_in6 *si6 = (struct sockaddr_in6*)ss;
        si6->sin6_port = htons(port);
        inet_pton(af, addr, &si6->sin6_addr);
    }else{
        printf("No support af(%d)\n", af);
    }
}

void test_sock_addr()
{
    // Test ipv4 address
    struct sockaddr_in ipv4_addr;

    ipv4_addr.sin_family = AF_INET;
    // Use htons convert port
    ipv4_addr.sin_port = htons(80);
    // use inet_addr
    ipv4_addr.sin_addr.s_addr = inet_addr("192.168.0.1");
    // use inet_aton
    inet_aton("192.168.0.1", &ipv4_addr.sin_addr);

    char *ip = inet_ntoa(ipv4_addr.sin_addr);

    printf("IPv4 Address after convert %s\n", ip);

    // Test ipv6 address
    struct sockaddr_in6 ipv6_addr;
    ipv6_addr.sin6_family = AF_INET6;
    ipv6_addr.sin6_port = htons(80);
    inet_pton(AF_INET6, "2001:0db8::0001", &ipv6_addr.sin6_addr);

    char v6addr[64] = {0};
    inet_ntop(AF_INET6, &ipv6_addr.sin6_addr, v6addr, 63);

    printf("IPv6 Address after convert %s\n", v6addr);

    // General sockaddr
    struct sockaddr_storage ss;

    init_sock_addr(AF_INET, &ss, 80, "192.168.0.1");
    init_sock_addr(AF_INET6, &ss, 80, "2001:168::1");
}

int main(int argc, char *argv[])
{
    test_sock_addr_length();
    test_sock_addr();

    return 0;
}

