#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netdb.h>
#include <arpa/inet.h>

static void my_addr_ntop(struct sockaddr *addr, char buf[], int len)
{
    if (addr->sa_family == AF_INET){
        inet_ntop(addr->sa_family, &((struct sockaddr_in*)addr)->sin_addr, buf, len);
    }else if (addr->sa_family == AF_INET6){
        inet_ntop(addr->sa_family, &((struct sockaddr_in6*)addr)->sin6_addr, buf, len);
    }
}


static void show_interface(int fd, const char *name)
{
    int family;
    struct ifreq ifreq;
    char host[128];

    memset(&ifreq, 0, sizeof ifreq);
    strncpy(ifreq.ifr_name, name, IFNAMSIZ);

    if(ioctl(fd, SIOCGIFADDR, &ifreq)!=0) {
        perror(name);
        return;
    }
    switch(family=ifreq.ifr_addr.sa_family) {
        case AF_INET:
        case AF_INET6:
            getnameinfo(&ifreq.ifr_addr, sizeof ifreq.ifr_addr, host, sizeof host, 0, 0, NI_NUMERICHOST);
            break;
        default:
            sprintf(host, "unknown (family: %d)", family);
    }
    printf("%-24s%s\n", name, host);
}

static void show_hardware_addr(int fd, const char *name)
{
    struct ifreq ifreq;

    memset(&ifreq, 0, sizeof ifreq);
    strncpy(ifreq.ifr_name, name, IFNAMSIZ);

    if(ioctl(fd, SIOCGIFHWADDR, &ifreq) != 0) {
        perror(name);
        return;
    }

    printf("Hardware addr:");
    if (ifreq.ifr_addr.sa_family == ARPHRD_ETHER){
        for (int i = 0; i < 6; ++i){
            unsigned char hex = (unsigned char)ifreq.ifr_addr.sa_data[i];
            printf("%02x ", hex);
        }
    }
    printf("\n");
}

static void show_flags(int fd, const char *name)
{
    struct ifreq ifreq;

    memset(&ifreq, 0, sizeof ifreq);
    strncpy(ifreq.ifr_name, name, IFNAMSIZ);

    if(ioctl(fd, SIOCGIFFLAGS, &ifreq) != 0) {
        perror(name);
        return;
    }

    short flags = ifreq.ifr_flags;
    printf("Flags:");
    if (flags & IFF_UP){
        printf("UP,");
    }

    if (flags & IFF_BROADCAST){
        printf("BROADCAST,");
    }

    if (flags & IFF_LOOPBACK){
        printf("LOOPBACK,");
    }

    if (flags & IFF_RUNNING){
        printf("RUNNING,");
    }

    if (flags & IFF_PROMISC){
        printf("PROMISC,");
    }

    if (flags & IFF_MULTICAST){
        printf("MULTICAST");
    }

    printf("\n");
}


static void list_interfaces(int fd, void (*show)(int fd, const char *name)) 
{
    struct ifreq *ifreq = NULL;
    struct ifconf ifconf;
    size_t len = 512;

    ifconf.ifc_buf = NULL;
    ifconf.ifc_len = len;

    while(1){
        if (ifconf.ifc_buf != NULL){
            free(ifconf.ifc_buf);
            len <<= 1;
        }

        ifconf.ifc_buf = malloc(len);
        ifconf.ifc_len = len;

        if(ioctl(fd, SIOCGIFCONF, &ifconf) == -1) {
            perror("ioctl(SIOCGIFCONF)");
            return;
        }

        if (ifconf.ifc_len < len)
            break;
    }

    int ITEMS = ifconf.ifc_len / sizeof(struct ifreq);

    for(int i = 0; i < ITEMS; ++i) {
        ifreq = &ifconf.ifc_req[i];
        char buf[128] = {0};
        my_addr_ntop(&ifreq->ifr_addr, buf, 127);
        printf("ifname:%s   ifaddr:%s\n", ifreq->ifr_name, buf);

        if (show){
            show(fd, ifreq->ifr_name);
        }
    }

    free(ifconf.ifc_buf);
}

void show_all_interfaces(int family)
{
    int fd = -1;

    fd=socket(family, SOCK_DGRAM, 0);
    if(fd<0) {
        perror("socket()");
        return;
    }

    list_interfaces(fd, show_interface);
    list_interfaces(fd, show_hardware_addr);
    list_interfaces(fd, show_flags);

    close(fd);
}

int main(int argc, char *argv[])
{
    show_all_interfaces(AF_INET); /* IPv4 */

    return 0;
}
