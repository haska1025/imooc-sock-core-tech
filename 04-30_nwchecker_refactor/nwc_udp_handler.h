#ifndef _NWC_UDP_HANDLER_H_
#define _NWC_UDP_HANDLER_H_

#include "nwc_io_handler.h"

struct nwc_udp_handler
{
    struct nwc_io_handler parent;
    int type;
};

int nwc_udp_handler_init(struct nwc_udp_handler *handler);

int nwc_udp_handler_listen(struct nwc_io_handler *handler, const char *ip, uint16_t port);
int nwc_udp_handler_connect(struct nwc_io_handler *handler, const char *hostname, uint16_t port);
int nwc_udp_handler_sendto(struct nwc_io_handler *handler, const uint8_t buffer[], uint32_t buflen, struct sockaddr *toaddr, socklen_t toaddrlen);
int nwc_udp_handler_recvfrom(struct nwc_io_handler *handler, uint8_t buffer[], uint32_t buflen, struct sockaddr *fromaddr, socklen_t *fromaddrlen);
int nwc_udp_handler_send(struct nwc_io_handler *handler, const uint8_t buffer[], uint32_t buflen);
int nwc_udp_handler_recv(struct nwc_io_handler *handler, uint8_t buffer[], uint32_t buflen);
int nwc_udp_handler_close(struct nwc_io_handler *handler);
void nwc_udp_handle_input(struct nwc_io_handler *handler);
void nwc_udp_handle_output(struct nwc_io_handler *handler);

#define udp_handler(io_handler) \
    (struct nwc_udp_handler*)(io_handler)


#endif//_NWC_UDP_HANDLER_H_

