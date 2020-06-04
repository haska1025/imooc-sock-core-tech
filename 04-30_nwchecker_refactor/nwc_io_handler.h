#ifndef _NWC_IO_HANDLER_H_
#define _NWC_IO_HANDLER_H_

#include "nwc_types.h"


#define EM_READ 1
#define EM_WRITE 2

struct nwc_io_handler
{
    void (*handle_input)(struct nwc_io_handler *handler);
    void (*handle_output)(struct nwc_io_handler *handler);

    nwc_handle_t fd;
};

#endif//_NWC_IO_HANDLER_H_

