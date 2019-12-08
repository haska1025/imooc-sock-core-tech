#include <stdio.h>

void check_endian()
{
    int n = 0xAABBCCDD;

    unsigned char *ptr_n = (unsigned char*)&n;

    for (int i=0; i < 4; ++i){
        printf("%X\n", *ptr_n++);
    }
}


int main(int argc, char *argv[])
{
    check_endian();

    return 0;
}

