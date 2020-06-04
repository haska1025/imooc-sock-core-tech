#ifndef _NWC_TCP_HANDLER_H_
#define _NWC_TCP_HANDLER_H_

#include "nwc_io_handler.h"

struct nwc_tcp_handler
{
    struct nwc_io_handler parent;
#define tcp_read parent.read
#define tcp_write paretn.write
};
#endif//_NWC_TCP_HANDLER_H_

