#include "packet.h"
#include <stdlib.h>

struct packet *packet_new(void)
{
    struct packet *packet = malloc(sizeof(*packet));
    if (!packet) return NULL;

    packet->msg = NULL;
    packet->arg = NULL;
    packet->result = 0;
    cco_node_init(&packet->node);

    return packet;
}

int packet_result(struct packet *packet) { return packet->result; }

void packet_del(struct packet *packet) { free(packet); }
