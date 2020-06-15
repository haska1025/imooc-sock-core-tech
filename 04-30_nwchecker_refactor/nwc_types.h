#ifndef _NWC_TYPES_H_
#define _NWC_TYPES_H_

#include <inttypes.h>
#include <stddef.h>
#include <netinet/in.h>

#define NWC_CLIENT 1
#define NWC_SERVER 2

#define NWC_PROTO_TCP 1
#define NWC_PROTO_UDP 2


typedef intptr_t nwc_handle_t;
#define INVALID_HANDLE (-1)

#define SCHEDULE_TIMER 50

#endif//_NWC_TYPES_H_

