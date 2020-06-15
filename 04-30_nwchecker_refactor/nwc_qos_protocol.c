#include "nwc_qos_protocol.h"
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static struct nwc_package_entry *nwc_qos_get_pkg_entry(struct list_head *hdr, uint32_t ack_ts)
{
    struct list_head *pos = NULL;
    struct nwc_package_entry *pkg_entry = NULL;

    list_for_each(pos, hdr){
        pkg_entry = list_entry(pos, struct nwc_package_entry, package_entry);
        if (pkg_entry->message->timestamp == ack_ts)
            return pkg_entry;
    }

    return NULL;
}

int nwc_qos_stat(struct nwc_qos_protocol *qos)
{
    uint32_t nowms = nwc_qos_milisecs();
    if (qos->last_stat_ts == 0){
        qos->last_stat_ts = nowms;
        return 0;
    }

    if ((int)(nowms - qos->last_stat_ts) >= qos->stat_interval){
        int secs = qos->stat_interval/1000;
        uint32_t bd = 0;
        double loss_rate =  (double)qos->loss_packages_per_secs/qos->sent_packages_per_secs;
        loss_rate /= secs;
        bd = (qos->sent_bytes * 8)/secs;

        printf("qos stat bandwidth(%u) rtt(%f) lossrate(%f)\n", bd, qos->rtt, loss_rate);

        qos->sent_bytes = 0;
        qos->last_stat_ts = nowms;
        qos->loss_packages_per_secs = 0;
        qos->sent_packages_per_secs = 0;
    }

    return 0;
}

int nwc_qos_init(struct nwc_qos_protocol *qos)
{
    qos->rtt = 0;
    qos->loss_packages_per_secs = 0;
    qos->sent_packages_per_secs = 0;
    qos->jitter = 0;
    qos->recv_timestamp = 0;
    qos->recv_time = 0;
    qos->sent_bytes = 0;
    qos->stat_interval = 1000;
    qos->last_stat_ts = 0;

    INIT_LIST_HEAD(&qos->sent_queue);
    return 0;
}

struct nwc_qos_hdr * nwc_qos_create_package(struct nwc_qos_protocol *qos, uint32_t pkglen)
{
    // Create package
    struct nwc_qos_hdr *pkg = malloc(sizeof(struct nwc_qos_hdr) + pkglen);
    pkg->ack_timestamp = 0;
    pkg->recv_delay = 0;
    pkg->length = pkglen;
    memcpy(pkg->data, "ping", 4);
    pkg->data[4] = 0;

    // Add to send queue
    struct nwc_package_entry *msg = malloc(sizeof(struct nwc_package_entry));
    msg->message = pkg;
    msg->flag = 0;
    list_add_tail(&msg->package_entry, &qos->sent_queue);

    qos->sent_packages_per_secs += 1;
    qos->sent_bytes += sizeof(struct nwc_qos_hdr) + pkglen;
//    pkg->timestamp = nwc_qos_milisecs();

    return pkg;
}

struct nwc_qos_hdr * nwc_qos_create_ack_package(struct nwc_qos_protocol *qos, uint32_t ack_ts)
{
     // Create package
    struct nwc_qos_hdr *pkg = malloc(sizeof(struct nwc_qos_hdr) + 4);
    pkg->ack_timestamp = ack_ts; 
    pkg->recv_delay = 0;
    pkg->length = 4;
    memcpy(pkg->data, "pong", 4);
    //pkg->timestamp = nwc_qos_milisecs();

    return pkg;
}

int nwc_qos_process_package(struct nwc_qos_protocol *qos, struct nwc_qos_hdr *pkg, uint32_t recv_ts)
{
    struct nwc_package_entry *entry = NULL;
    double rtt_sample = 0;

    if (pkg->ack_timestamp == 0)
        return 0;

    entry = nwc_qos_get_pkg_entry(&qos->sent_queue, pkg->ack_timestamp);
    if (!entry){
        // inorder
        return 0;
    }
#ifdef _DEBUG
    printf("nwc_qos_process rtt(%f) recv_delay(%u) recv_ts(%u) ack_ts(%u)\n",
            qos->rtt,
            pkg->recv_delay,
            recv_ts,
            pkg->ack_timestamp);
#endif
    rtt_sample = recv_ts - pkg->ack_timestamp;
    rtt_sample -= pkg->recv_delay;

    qos->rtt = qos->rtt * 0.875 + rtt_sample * 0.125;

    // Remove from sent queue
    list_del(&entry->package_entry);
    
    free(entry->message);
    free(entry);

    return 0;
}

uint32_t nwc_qos_milisecs()
{
    struct timespec ts;
    uint64_t ticks = 0;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    ticks = (uint64_t)ts.tv_sec * 1000 + (uint64_t)(ts.tv_nsec) / 1000;

    return ticks;
}

