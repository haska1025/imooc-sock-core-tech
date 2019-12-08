#include <stdio.h>
#include <arpa/inet.h>

void show_bytes(int n)
{
    unsigned char *ptr_n = (unsigned char*)&n;

    for (int i = 0; i < 4; ++i){
        printf("%X\n", *ptr_n++);
    }
}

void byte_order_convert()
{
    int x = 0xAABBCCDD;

    printf("Before byte order convert\n");
    show_bytes(x);

    int ax = htonl(x);
    printf("After byte order convert\n");
    show_bytes(ax);
}

int main(int argc, char *argv[])
{
    byte_order_convert();
    return 0;
}

