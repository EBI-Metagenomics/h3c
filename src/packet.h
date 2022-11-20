#ifndef PACKET_H
#define PACKET_H

#include "cco.h"

struct msg;

struct packet
{
    struct msg *msg;
    void *arg;
    int result;
    struct cco_node node;
};

struct packet *packet_new(void);
void packet_del(struct packet *);

#endif
