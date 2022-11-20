#ifndef PACKET_H
#define PACKET_H

#include "cco.h"

struct msg;

struct packet
{
    struct msg *msg;
    int rc;
    struct cco_node node;
};

struct packet *packet_new(void);
void packet_del(struct packet *);

#endif
