#include <stdio.h>
#include "nwc_looper.h"

int main(int argc, char *argv[])
{
    nwc_handle_t main_looper = nwc_looper_create();

    nwc_looper_destroy(main_looper);
    return 0;
}

