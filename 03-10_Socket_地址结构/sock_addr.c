#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>


void test_sock_addr_length()
{
    printf("The length of struct sockaddr_in: %d\n", sizeof(struct sockaddr_in));
    printf("The length of struct sockaddr_in6: %d\n", sizeof(struct sockaddr_in6));
    printf("The length of struct sockaddr_un: %d\n", sizeof(struct sockaddr_un));
    printf("The length of struct sockaddr: %d\n", sizeof(struct sockaddr));
    printf("The length of struct sockaddr_storage: %d\n", sizeof(struct sockaddr_storage));
}

int main(int argc, char *argv[])
{
    test_sock_addr_length();
    return 0;
}

