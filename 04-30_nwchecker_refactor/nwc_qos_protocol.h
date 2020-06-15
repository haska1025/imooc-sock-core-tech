#ifndef _NWC_QOS_PROTOCOL_H_
#define _NWC_QOS_PROTOCOL_H_

#include <inttypes.h>

#include "list.h"

struct nwc_qos_hdr
{
    uint32_t timestamp; // Send timestamp
    uint32_t ack_timestamp; // Ack timestamp
    uint32_t recv_delay; // The delay for receiver process message
    uint32_t length; // The length of the payload
    uint8_t data[0]; // dummy field
};

#define LOSS_FLAG 1
#define INORDER_FLAG 2

// The package wrapper for saving
struct nwc_package_entry
{
    struct nwc_qos_hdr *message;
    int flag;
    struct list_head package_entry;
};

struct nwc_qos_protocol
{
    double rtt;
    uint32_t loss_packages_per_secs;
    uint32_t sent_packages_per_secs; 
    int jitter;
    uint32_t recv_timestamp;
    uint32_t recv_time;
    uint32_t sent_bytes;
    uint32_t send_bandwidth;
    uint32_t last_stat_ts;
    int stat_interval;

    struct list_head sent_queue;
};

int nwc_qos_init(struct nwc_qos_protocol *qos);
struct nwc_qos_hdr * nwc_qos_create_package(struct nwc_qos_protocol *qos, uint32_t packagelen); 
struct nwc_qos_hdr * nwc_qos_create_ack_package(struct nwc_qos_protocol *qos, uint32_t ack_ts); 
int nwc_qos_process_package(struct nwc_qos_protocol *qos, struct nwc_qos_hdr *pkg, uint32_t recv_ts);
int nwc_qos_stat(struct nwc_qos_protocol *qos);

uint32_t nwc_qos_milisecs();

#endif//_NWC_QOS_PROTOCOL_H_

