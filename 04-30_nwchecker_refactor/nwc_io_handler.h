#ifndef _NWC_IO_HANDLER_H_
#define _NWC_IO_HANDLER_H_

#define EM_READ 1
#define EM_WRITE 2

struct nwc_io_handler
{
    void (*handle_input)(struct nwc_io_handler *handler);
    void (*handle_output)(struct nwc_io_handler *handler);

    int fd;
};

#endif//_NWC_IO_HANDLER_H_

